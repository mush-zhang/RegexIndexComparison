#include "trigram_inverted_index.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <algorithm>
#include <chrono>
#include "../../utils/utils.hpp"

void trigram_index::TrigramInvertedIndex::extract_trigrams(const std::string & line, std::set<std::string> & trigrams) const {
    if (line.size() < 3) return;
    for (size_t i = 0; i + 3 <= line.size(); ++i) {
        trigrams.insert(line.substr(i, 3));
    }
}

void trigram_index::TrigramInvertedIndex::build_index(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();

    // Step 1: Collect all unique trigrams in the dataset (threaded)
    const size_t num_threads = thread_count_; // std::thread::hardware_concurrency();
    std::vector<std::set<std::string>> thread_trigrams(num_threads);
    size_t dataset_size = k_dataset_.size();
    size_t local_limit = (key_upper_bound_ < LLONG_MAX) ? (5 * key_upper_bound_ + num_threads - 1) / num_threads : std::numeric_limits<size_t>::max();

    auto collect_trigrams = [&](size_t tid) {
        size_t chunk = (dataset_size + num_threads - 1) / num_threads;
        size_t start = tid * chunk;
        size_t end = std::min(start + chunk, dataset_size);
        for (size_t i = start; i < end; ++i) {
            if (thread_trigrams[tid].size() >= local_limit) break;
                std::set<std::string> local;
                extract_trigrams(k_dataset_[i], local);
                for (const auto& tri : local) {
                    if (thread_trigrams[tid].size() < local_limit)
                        thread_trigrams[tid].insert(tri);
                    else
                        break;
            }
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < num_threads; ++t)
        threads.emplace_back(collect_trigrams, t);
    for (auto &th : threads) th.join();

    // Merge all trigrams
    std::set<std::string> all_trigrams;
    for (const auto &local_set : thread_trigrams)
        all_trigrams.insert(local_set.begin(), local_set.end());

    if (key_upper_bound_ < LLONG_MAX) {
        // Randomly select key_upper_bound_ trigrams from all_trigrams
        std::vector<std::string> trigram_vec(all_trigrams.begin(), all_trigrams.end());
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(trigram_vec.begin(), trigram_vec.end(), g);

        k_index_keys_.clear();
        for (int i = 0; i < key_upper_bound_ && i < static_cast<int>(trigram_vec.size()); ++i) {
            k_index_keys_.insert(trigram_vec[i]);
        }
    } else {
        k_index_keys_ = all_trigrams;
    }
    auto selection_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << selection_time << " s" << std::endl;

    std::ostringstream log;
    log << "Trigram," << thread_count_ << "," << upper_n << ",";
    log << -1 << "," << key_upper_bound_ << ",";
    log << k_queries_size_ << "," << selection_time << ",";

    // Step 2: Fill posting lists (threaded)
    fill_posting();

    auto build_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << build_time << std::endl;
    log << build_time << "," << build_time+selection_time << ",";
    log << get_num_keys() << "," << get_bytes_used() << ",";

    write_to_file(log.str());
}

void trigram_index::TrigramInvertedIndex::fill_posting() {
    const size_t num_threads = thread_count_; // std::thread::hardware_concurrency();
    size_t dataset_size = k_dataset_.size();

    auto fill = [&](size_t tid) {
        size_t chunk = (dataset_size + num_threads - 1) / num_threads;
        size_t start = tid * chunk;
        size_t end = std::min(start + chunk, dataset_size);

        std::unordered_map<std::string, std::vector<size_t>> local_index;
        for (size_t i = start; i < end; ++i) {
            const std::string & line = k_dataset_[i];
            if (line.size() < 3) continue;
            for (size_t j = 0; j + 3 <= line.size(); ++j) {
                std::string trigram = line.substr(j, 3);
                if (k_index_keys_.count(trigram))
                    local_index[trigram].push_back(i);
            }
        }
        std::cout << "Thread " << tid << " processed lines from " << start << " to " << end << std::endl;
        // Merge local_index into global index
        std::lock_guard<std::mutex> lock(index_mutex_);
        for (const auto & [key, vec] : local_index) {
            auto & posting = k_index_[key];
            posting = sorted_lists_union(posting, vec);
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < num_threads; ++t)
        threads.emplace_back(fill, t);
    for (auto &th : threads) th.join();
}
