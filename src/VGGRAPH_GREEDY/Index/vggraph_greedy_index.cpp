#include "vggraph_greedy_index.hpp"
#include <unordered_set>
#include <thread>
#include <future>
#include <iostream>
#include <algorithm>
#include <limits>

namespace vggraph_greedy_index {

void VGGraph_Greedy::build_index(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_n);
    auto selection_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << selection_time << " s" << std::endl;
    
    std::ostringstream log;
    log << "VGGraph-Greedy" << "," << thread_count_ << "," << upper_n << ",";
    log << k_threshold_ << "," << key_upper_bound_ << "," << k_queries_size_ << ",";
    log << selection_time << ",";

    auto build_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Index Building End in " << build_time << std::endl;

    log << build_time << "," << build_time+selection_time << ",";
    log << get_num_keys() << "," << get_bytes_used() << ",";
    write_to_file(log.str());
}

void VGGraph_Greedy::select_grams(int upper_n) {
    if (upper_n == -1) upper_n = upper_n_;
    max_gram_len_ = upper_n;
    
    // Step 1: Build initial q_min-grams for dynamic tau computation
    std::unordered_map<std::string, PostingList> gram2posting;
    build_initial_ngrams_parallel(gram2posting);
    
    // Step 2: Compute dynamic tau based on initial gram frequencies
    size_t tau = dynamic_tau(gram2posting, 0.8); // 80th percentile
    std::cout << "Dynamic tau = " << tau << " (threshold = " << k_threshold_ << ")" << std::endl;
    
    // Step 3: Build VGram index with recursive extension
    std::set<std::string> temp_index_keys;
    std::unordered_map<std::string, PostingList> temp_index;
    build_vgram_index_parallel(tau, temp_index_keys, temp_index);
    
    std::cout << "Total grams before set cover: " << temp_index_keys.size() << std::endl;
    
    // Step 4: Apply set cover optimization if queries are available
    if (k_queries_size_ > 0) {
        auto query_literals = get_query_literals();
        std::set<std::string> selected_grams_set;
        
        // Apply set cover for each query and collect selected grams
        for (const auto& literals : query_literals) {
            auto selected = vggraph_greedy_cover(literals, temp_index_keys, temp_index);
            selected_grams_set.insert(selected.begin(), selected.end());
        }
        
        // Build final index with only selected grams
        for (const auto& gram : selected_grams_set) {
            auto it = temp_index.find(gram);
            if (it != temp_index.end()) {
                k_index_[gram] = it->second;
                k_index_keys_.insert(gram);
                
                if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                    break;
                }
            }
        }
        
        std::cout << "Selected grams after set cover: " << k_index_keys_.size() << std::endl;
    } else {
        // No queries available, use all generated grams
        std::cout << "No queries provided, using all generated grams" << std::endl;
        for (const auto& gram : temp_index_keys) {
            k_index_[gram] = temp_index[gram];
            k_index_keys_.insert(gram);
            
            if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                break;
            }
        }
    }
}

void VGGraph_Greedy::recursive_extend(
    const std::string& gram,
    const PostingList& rec_ids,
    size_t tau,
    std::set<std::string>& index_keys,
    std::unordered_map<std::string, PostingList>& index_map) {
    
    if (gram.size() < q_min_) return;
    
    if (rec_ids.size() <= tau || gram.size() >= max_gram_len_) {
        index_keys.insert(gram);
        index_map[gram] = rec_ids;
        return;
    }
    
    std::unordered_map<std::string, PostingList> ext_map;
    for (RecordId rec_id : rec_ids) {
        const std::string& rec = k_dataset_[rec_id];
        for (size_t pos = 0; pos + gram.size() < rec.size(); ++pos) {
            if (rec.substr(pos, gram.size()) == gram) {
                std::string ext_gram = gram + rec[pos + gram.size()];
                ext_map[ext_gram].push_back(rec_id);
            }
        }
    }
    
    for (auto& kv : ext_map) {
        std::sort(kv.second.begin(), kv.second.end());
        kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
        recursive_extend(kv.first, kv.second, tau, index_keys, index_map);
    }
}

void VGGraph_Greedy::build_vgram_index_parallel(
    size_t tau,
    std::set<std::string>& index_keys,
    std::unordered_map<std::string, PostingList>& index_map) {
    
    size_t N = k_dataset_size_;
    size_t chunk = (N + thread_count_ - 1) / thread_count_;
    std::vector<std::future<void>> futures;
    std::vector<std::set<std::string>> thread_keys(thread_count_);
    std::vector<std::unordered_map<std::string, PostingList>> thread_index(thread_count_);

    for (int t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk;
        size_t end = std::min(N, (t + 1) * chunk);
        if (start >= end) break;
        
        futures.emplace_back(std::async(std::launch::async, [this, &thread_keys, &thread_index, t, start, end, tau]() {
            std::unordered_map<std::string, PostingList> initial_grams;
            
            // Build initial q_min-grams for this chunk
            for (RecordId rec_id = start; rec_id < end; ++rec_id) {
                const std::string& rec = k_dataset_[rec_id];
                for (size_t i = 0; i + q_min_ <= rec.size(); ++i) {
                    std::string gram = rec.substr(i, q_min_);
                    initial_grams[gram].push_back(rec_id);
                }
            }
            
            // Apply recursive extension for each initial gram
            for (auto& kv : initial_grams) {
                std::sort(kv.second.begin(), kv.second.end());
                kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
                recursive_extend(kv.first, kv.second, tau, thread_keys[t], thread_index[t]);
            }
        }));
    }
    
    // Wait for all threads to complete
    for (auto& fut : futures) {
        fut.get();
    }

    // Merge results from all threads
    for (int t = 0; t < thread_count_; ++t) {
        merge_index_maps(index_keys, index_map, thread_keys[t], thread_index[t]);
    }
    
    // De-duplicate all posting lists
    for (auto& kv : index_map) {
        std::sort(kv.second.begin(), kv.second.end());
        kv.second.erase(std::unique(kv.second.begin(), kv.second.end()), kv.second.end());
    }
}

void VGGraph_Greedy::merge_index_maps(
    std::set<std::string>& out_keys,
    std::unordered_map<std::string, PostingList>& out_index,
    const std::set<std::string>& in_keys,
    const std::unordered_map<std::string, PostingList>& in_index) {
    
    for (const auto& gram : in_keys) {
        out_keys.insert(gram);
        auto& plist = out_index[gram];
        auto it = in_index.find(gram);
        if (it != in_index.end()) {
            plist.insert(plist.end(), it->second.begin(), it->second.end());
        }
    }
}

std::vector<std::string> VGGraph_Greedy::vggraph_greedy_cover(
    const std::vector<std::string>& literals,
    const std::set<std::string>& index_keys,
    const std::unordered_map<std::string, PostingList>& index_map) {
    
    size_t total_chars = 0;
    std::vector<size_t> lit_offsets;
    for (const auto& lit : literals) {
        lit_offsets.push_back(total_chars);
        total_chars += lit.size();
    }
    
    std::unordered_map<std::string, std::vector<size_t>> gram2cover;
    for (const auto& gram : index_keys) {
        std::vector<size_t> covered;
        for (size_t li = 0; li < literals.size(); ++li) {
            const std::string& lit = literals[li];
            for (size_t i = 0; i + gram.size() <= lit.size(); ++i) {
                if (lit.substr(i, gram.size()) == gram) {
                    for (size_t k = 0; k < gram.size(); ++k) {
                        covered.push_back(lit_offsets[li] + i + k);
                    }
                }
            }
        }
        if (!covered.empty()) {
            gram2cover[gram] = std::move(covered);
        }
    }
    
    std::vector<bool> covered(total_chars, false);
    size_t uncovered = total_chars;
    std::vector<std::string> selected;
    
    while (uncovered > 0) {
        double best_score = std::numeric_limits<double>::max();
        std::string best_gram;
        
        for (const auto& kv : gram2cover) {
            const auto& gram = kv.first;
            auto index_it = index_map.find(gram);
            if (index_it == index_map.end()) continue;
            
            size_t cost = index_it->second.size();
            size_t gain = 0;
            for (size_t pos : kv.second) {
                if (!covered[pos]) ++gain;
            }
            if (gain == 0) continue;
            
            double score = static_cast<double>(cost) / gain;
            if (score < best_score) {
                best_score = score;
                best_gram = gram;
            }
        }
        
        if (best_gram.empty()) break;
        
        for (size_t pos : gram2cover[best_gram]) {
            if (!covered[pos]) {
                covered[pos] = true;
                --uncovered;
            }
        }
        selected.push_back(best_gram);
    }
    
    return selected;
}

size_t VGGraph_Greedy::dynamic_tau(
    const std::unordered_map<std::string, PostingList>& grams, 
    double quantile) {
    
    std::vector<size_t> sizes;
    for (const auto& kv : grams) {
        sizes.push_back(kv.second.size());
    }
    std::sort(sizes.begin(), sizes.end());
    
    if (sizes.empty()) return 10;
    
    size_t idx = static_cast<size_t>(sizes.size() * quantile);
    if (idx >= sizes.size()) idx = sizes.size() - 1;
    
    return sizes[idx];
}

void VGGraph_Greedy::build_initial_ngrams_parallel(
    std::unordered_map<std::string, PostingList>& initial_grams) {
    
    std::vector<std::unordered_map<std::string, PostingList>> thread_grams(thread_count_);
    std::vector<std::thread> threads;
    
    size_t chunk_size = (k_dataset_size_ + thread_count_ - 1) / thread_count_;
    
    for (int t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, k_dataset_size_);
        if (start >= end) break;
        
        threads.emplace_back([this, &thread_grams, t, start, end]() {
            process_chunk_for_initial_grams(start, end, thread_grams[t]);
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Merge results from all threads
    for (const auto& local_grams : thread_grams) {
        for (const auto& entry : local_grams) {
            const std::string& gram = entry.first;
            const PostingList& positions = entry.second;
            
            initial_grams[gram].insert(initial_grams[gram].end(), positions.begin(), positions.end());
        }
    }
    
    // Sort and remove duplicates for each gram
    for (auto& entry : initial_grams) {
        auto& vec = entry.second;
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
}

void VGGraph_Greedy::process_chunk_for_initial_grams(
    size_t start, size_t end,
    std::unordered_map<std::string, PostingList>& thread_grams) {
    
    for (RecordId rec_id = start; rec_id < end; ++rec_id) {
        const std::string& rec = k_dataset_[rec_id];
        for (size_t i = 0; i + q_min_ <= rec.size(); ++i) {
            std::string gram = rec.substr(i, q_min_);
            thread_grams[gram].push_back(rec_id);
        }
    }
}

} // namespace vggraph_greedy_index
