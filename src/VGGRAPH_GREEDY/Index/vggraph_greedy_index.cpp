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
    std::unordered_map<std::string, PostingList> current_grams;
    build_initial_ngrams_parallel(current_grams);
    
    // Step 2: Compute dynamic tau based on initial gram frequencies using k_threshold_ as quantile
    size_t tau = 0;
    if (k_threshold_ < 0) {
        tau = dynamic_tau(current_grams, 0.8);
    }
    else {
        tau = static_cast<size_t>(k_threshold_ * k_dataset_size_);
    }
    // Step 3: Iterative pruning approach
    std::set<std::string> selected_grams_cumulative;
    
    for (size_t current_len = q_min_; current_len <= max_gram_len_; ++current_len) {        
        // Filter grams by frequency threshold (tau)
        std::unordered_map<std::string, PostingList> filtered_grams;
        for (const auto& entry : current_grams) {
            if (entry.second.size() <= tau) {
                filtered_grams[entry.first] = entry.second;
            }
        }
                
        // Apply set cover optimization if queries are available
        if (k_queries_size_ > 0 && !filtered_grams.empty()) {
            auto query_literals = get_query_literals();
            std::set<std::string> gram_keys;
            for (const auto& entry : filtered_grams) {
                gram_keys.insert(entry.first);
            }
            
            std::set<std::string> selected_this_round;
            
            // Apply set cover for each query and collect selected grams
            for (const auto& literals : query_literals) {
                auto selected = vggraph_greedy_cover(literals, gram_keys, filtered_grams);
                selected_this_round.insert(selected.begin(), selected.end());
            }
                        
            // Add selected grams to cumulative set
            selected_grams_cumulative.insert(selected_this_round.begin(), selected_this_round.end());
            
            // Add selected grams to final index
            for (const auto& gram : selected_this_round) {
                auto it = filtered_grams.find(gram);
                if (it != filtered_grams.end()) {
                    k_index_[gram] = it->second;
                    k_index_keys_.insert(gram);
                    
                    if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                        return;
                    }
                }
            }
        } else {
            // No queries available, use all filtered grams
            for (const auto& entry : filtered_grams) {
                k_index_[entry.first] = entry.second;
                k_index_keys_.insert(entry.first);
                selected_grams_cumulative.insert(entry.first);
                
                if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                    return;
                }
            }
        }
        
        // Step 4: Extend only the selected grams for next iteration
        if (current_len < max_gram_len_) {
            std::unordered_map<std::string, PostingList> next_grams;
            extend_selected_grams(current_grams, selected_grams_cumulative, next_grams, tau);
            current_grams = std::move(next_grams);
                        
            if (current_grams.empty()) {
                break;
            }
        }
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

void VGGraph_Greedy::extend_selected_grams(
    const std::unordered_map<std::string, PostingList>& current_grams,
    const std::set<std::string>& selected_grams,
    std::unordered_map<std::string, PostingList>& next_grams,
    size_t tau) {
    
    // Find which grams need extension (those that are too frequent)
    std::set<std::string> grams_to_extend;
    for (const auto& entry : current_grams) {
        const std::string& gram = entry.first;
        const PostingList& positions = entry.second;
        
        // Only extend if:
        // 1. The gram is too frequent (> tau), OR
        // 2. The gram was selected in previous rounds (to allow further extension)
        if (positions.size() > tau || selected_grams.find(gram) != selected_grams.end()) {
            grams_to_extend.insert(gram);
        }
    }
        
    if (!grams_to_extend.empty()) {
        extend_grams_parallel(current_grams, grams_to_extend, next_grams, tau);
    }
}

void VGGraph_Greedy::extend_grams_parallel(
    const std::unordered_map<std::string, PostingList>& current_grams,
    const std::set<std::string>& grams_to_extend,
    std::unordered_map<std::string, PostingList>& extended_grams,
    size_t tau) {
    
    std::vector<std::string> grams_vec(grams_to_extend.begin(), grams_to_extend.end());
    std::vector<std::unordered_map<std::string, PostingList>> thread_results(thread_count_);
    std::vector<std::thread> threads;
    
    size_t chunk_size = (grams_vec.size() + thread_count_ - 1) / thread_count_;
    
    for (int t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, grams_vec.size());
        if (start >= end) break;
        
        threads.emplace_back([this, &grams_vec, &current_grams, &thread_results, t, start, end]() {
            auto& local_extended = thread_results[t];
            
            for (size_t i = start; i < end; ++i) {
                const std::string& gram = grams_vec[i];
                auto gram_it = current_grams.find(gram);
                if (gram_it == current_grams.end()) continue;
                
                const PostingList& positions = gram_it->second;
                
                // Extend this gram by one character
                std::unordered_map<std::string, PostingList> local_extensions;
                
                for (RecordId rec_id : positions) {
                    const std::string& rec = k_dataset_[rec_id];
                    
                    // Find all occurrences of the gram in this record
                    for (size_t pos = 0; pos + gram.size() < rec.size(); ++pos) {
                        if (rec.substr(pos, gram.size()) == gram) {
                            // Extend with the next character
                            std::string ext_gram = gram + rec[pos + gram.size()];
                            local_extensions[ext_gram].push_back(rec_id);
                        }
                    }
                }
                
                // Add valid extensions to thread results
                for (auto& ext_entry : local_extensions) {
                    auto& ext_positions = ext_entry.second;
                    std::sort(ext_positions.begin(), ext_positions.end());
                    ext_positions.erase(std::unique(ext_positions.begin(), ext_positions.end()), ext_positions.end());
                    
                    // Only keep extensions that have at least one occurrence
                    if (!ext_positions.empty()) {
                        local_extended[ext_entry.first] = std::move(ext_positions);
                    }
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Merge results from all threads
    for (const auto& thread_result : thread_results) {
        for (const auto& entry : thread_result) {
            const std::string& gram = entry.first;
            const PostingList& positions = entry.second;
            
            extended_grams[gram].insert(extended_grams[gram].end(), positions.begin(), positions.end());
        }
    }
    
    // Sort and remove duplicates for each extended gram
    for (auto& entry : extended_grams) {
        auto& vec = entry.second;
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
}

} // namespace vggraph_greedy_index
