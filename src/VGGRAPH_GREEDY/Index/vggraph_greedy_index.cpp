#include "vggraph_greedy_index.hpp"
#include <unordered_set>
#include <thread>
#include <future>
#include <iostream>
#include <algorithm>

namespace vggraph_greedy_index {

void VGGraph_Greedy::build_index(int upper_n) {
    if (upper_n == -1) upper_n = upper_n_;
    max_gram_len_ = upper_n;
    
    find_alphabet();
    
    // Start with minimum n-grams using parallel processing
    std::unordered_map<std::string, std::vector<size_t>> next_index;
    std::set<std::string> current_grams;

    // Build initial n-grams in parallel
    build_initial_ngrams_parallel(next_index, current_grams);

    // Convert selectivity threshold to tau (document count threshold)
    size_t tau = static_cast<size_t>(selectivity_threshold_ * k_dataset_size_);

    int current_len = q_min_;
    while (!current_grams.empty()) {
        std::unordered_map<std::string, std::vector<size_t>> new_index;
        std::set<std::string> next_grams;

        // Process current grams in parallel
        process_grams_parallel(current_grams, next_index, new_index, next_grams, tau, current_len);

        // Move to next iteration
        if (++current_len > max_gram_len_) {
            // At max length, only filter without extending
            for (const auto& gram : current_grams) {
                const auto& positions = next_index[gram];
                if (positions.size() <= tau) {
                    k_index_[gram] = positions;
                    k_index_keys_.insert(gram);
                    
                    if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                        return;
                    }
                }
            }
            break; // Final cutoff
        }

        current_grams = std::move(next_grams);
        next_index = std::move(new_index);
    }
}

bool VGGraph_Greedy::get_all_idxs(const std::string & reg, std::vector<size_t> & container) const {
    container.clear();
    
    auto keys = find_all_keys(reg);
    std::set<size_t> result_set;
    
    for (const auto& key : keys) {
        const auto& positions = get_line_pos_at(key);
        result_set.insert(positions.begin(), positions.end());
    }
    
    container.assign(result_set.begin(), result_set.end());
    return !container.empty();
}

void VGGraph_Greedy::select_grams(int upper_n) {
    // This method is called by build_index, the actual selection logic is there
    // Following the reference implementation pattern
}

void VGGraph_Greedy::find_alphabet() {
    std::unordered_set<char> char_set;
    for (const auto& document : k_dataset_) {
        for (char c : document) {
            if (std::isprint(c)) {
                char_set.insert(c);
            }
        }
    }
    alphabet_.assign(char_set.begin(), char_set.end());
    std::sort(alphabet_.begin(), alphabet_.end());
}

void VGGraph_Greedy::build_initial_ngrams_parallel(
    std::unordered_map<std::string, std::vector<size_t>>& next_index,
    std::set<std::string>& current_grams) {
    
    // Thread-safe containers for collecting results
    std::vector<std::unordered_map<std::string, std::vector<size_t>>> thread_indices(thread_count_);
    std::vector<std::thread> threads;
    
    // Divide work among threads
    size_t chunk_size = (k_dataset_size_ + thread_count_ - 1) / thread_count_;
    
    for (int t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, k_dataset_size_);
        if (start >= end) break;
        
        threads.emplace_back([this, &thread_indices, t, start, end]() {
            auto& local_index = thread_indices[t];
            for (size_t i = start; i < end; ++i) {
                std::string text = k_dataset_[i];
                if (append_terminal_) text += "$";
                
                for (size_t j = 0; j + q_min_ <= text.size(); ++j) {
                    std::string gram = text.substr(j, q_min_);
                    local_index[gram].push_back(i);
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Merge results from all threads
    for (const auto& local_index : thread_indices) {
        for (const auto& entry : local_index) {
            const std::string& gram = entry.first;
            const std::vector<size_t>& positions = entry.second;
            
            next_index[gram].insert(next_index[gram].end(), positions.begin(), positions.end());
            current_grams.insert(gram);
        }
    }
    
    // Sort and remove duplicates for each gram
    for (auto& entry : next_index) {
        auto& vec = entry.second;
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
}

void VGGraph_Greedy::process_grams_parallel(
    const std::set<std::string>& current_grams,
    const std::unordered_map<std::string, std::vector<size_t>>& next_index,
    std::unordered_map<std::string, std::vector<size_t>>& new_index,
    std::set<std::string>& next_grams,
    size_t tau,
    int current_len) {
    
    std::mutex new_index_mutex;
    std::mutex next_grams_mutex;
    std::mutex final_index_mutex;
    
    // Convert grams to vector for parallel processing
    std::vector<std::string> grams_vec(current_grams.begin(), current_grams.end());
    std::vector<std::thread> threads;
    
    size_t chunk_size = (grams_vec.size() + thread_count_ - 1) / thread_count_;
    
    for (int t = 0; t < thread_count_; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, grams_vec.size());
        if (start >= end) break;
        
        threads.emplace_back([this, &grams_vec, &next_index, &new_index, &next_grams, 
                             &new_index_mutex, &next_grams_mutex, &final_index_mutex,
                             start, end, tau, current_len]() {
            
            // Local containers for this thread
            std::unordered_map<std::string, std::vector<size_t>> local_new_index;
            std::set<std::string> local_next_grams;
            
            for (size_t i = start; i < end; ++i) {
                const std::string& gram = grams_vec[i];
                const auto& positions = next_index.at(gram);
                
                if (positions.size() > tau) {
                    // Too frequent - extend if possible
                    if (current_len < max_gram_len_) {
                        for (char c : alphabet_) {
                            std::string ext_gram = gram + c;
                            
                            // Only check documents where the prefix gram exists
                            for (size_t doc_id : positions) {
                                std::string text = k_dataset_[doc_id];
                                if (append_terminal_) text += "$";
                                
                                // Check if extended gram exists in this document
                                if (text.find(ext_gram) != std::string::npos) {
                                    local_new_index[ext_gram].push_back(doc_id);
                                }
                            }
                            
                            // Remove duplicates and sort
                            if (!local_new_index[ext_gram].empty()) {
                                auto& vec = local_new_index[ext_gram];
                                std::sort(vec.begin(), vec.end());
                                vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
                                local_next_grams.insert(ext_gram);
                            }
                        }
                    }
                } else {
                    // Acceptable gram - keep it in final index
                    std::lock_guard<std::mutex> lock(final_index_mutex);
                    k_index_[gram] = positions;
                    k_index_keys_.insert(gram);
                    
                    // Apply key upper bound if set
                    if (key_upper_bound_ > 0 && k_index_keys_.size() >= static_cast<size_t>(key_upper_bound_)) {
                        return; // This thread stops when limit is reached
                    }
                }
            }
            
            // Merge local results into global containers
            {
                std::lock_guard<std::mutex> lock(new_index_mutex);
                for (auto& entry : local_new_index) {
                    const std::string& gram = entry.first;
                    auto& vec = entry.second;
                    
                    new_index[gram].insert(new_index[gram].end(), vec.begin(), vec.end());
                }
            }
            
            {
                std::lock_guard<std::mutex> lock(next_grams_mutex);
                next_grams.insert(local_next_grams.begin(), local_next_grams.end());
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Final cleanup: sort and remove duplicates for extended grams
    for (auto& entry : new_index) {
        auto& vec = entry.second;
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    }
}

} // namespace vggraph_greedy_index
