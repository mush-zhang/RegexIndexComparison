#include "trigram_inverted_index.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

void trigram_index::TrigramInvertedIndex::extract_trigrams(const std::string & line, std::set<std::string> & trigrams) const {
    if (line.size() < 3) return;
    for (size_t i = 0; i + 3 <= line.size(); ++i) {
        trigrams.insert(line.substr(i, 3));
    }
}

void trigram_index::TrigramInvertedIndex::build_index(int upper_n) {
    // Step 1: Collect all unique trigrams in the dataset (threaded)
    const size_t num_threads = thread_count_; // std::thread::hardware_concurrency();
    std::vector<std::set<std::string>> thread_trigrams(num_threads);
    size_t dataset_size = k_dataset_.size();

    auto collect_trigrams = [&](size_t tid) {
        size_t chunk = (dataset_size + num_threads - 1) / num_threads;
        size_t start = tid * chunk;
        size_t end = std::min(start + chunk, dataset_size);
        for (size_t i = start; i < end; ++i) {
            extract_trigrams(k_dataset_[i], thread_trigrams[tid]);
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < num_threads; ++t)
        threads.emplace_back(collect_trigrams, t);
    for (auto &th : threads) th.join();

    // Merge all trigrams into k_index_keys_
    for (const auto &local_set : thread_trigrams)
        k_index_keys_.insert(local_set.begin(), local_set.end());

    // Step 2: Fill posting lists (threaded)
    fill_posting();
}

void trigram_index::TrigramInvertedIndex::fill_posting() {
    const size_t num_threads = std::thread::hardware_concurrency();
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
        // Merge local_index into global index
        std::lock_guard<std::mutex> lock(index_mutex_);
        for (const auto & [key, vec] : local_index) {
            auto & posting = k_index_[key];
            posting.insert(posting.end(), vec.begin(), vec.end());
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < num_threads; ++t)
        threads.emplace_back(fill, t);
    for (auto &th : threads) th.join();
}

std::vector<std::string> trigram_index::TrigramInvertedIndex::find_all_keys(const std::string & line) const {
    std::set<std::string> trigrams;
    extract_trigrams(line, trigrams);
    std::vector<std::string> found;
    for (const auto & t : trigrams) {
        if (k_index_keys_.count(t)) found.push_back(t);
    }
    return found;
}

void trigram_index::TrigramInvertedIndex::print_index(bool size_only) const {
    std::cout << "TrigramInvertedIndex: " << k_index_keys_.size() << " keys" << std::endl;
    if (!size_only) {
        for (const auto & key : k_index_keys_) {
            std::cout << key << ": ";
            if (k_index_.count(key))
                std::cout << k_index_.at(key).size() << " lines" << std::endl;
            else
                std::cout << "0 lines" << std::endl;
        }
    }
}

void trigram_index::TrigramInvertedIndex::wirte_index_keys_to_file(const std::filesystem::path & out_path) const {
    std::ofstream outfile(out_path);
    for (const auto & k : k_index_keys_) {
        outfile << k << std::endl;
    }
}

const std::vector<size_t> & trigram_index::TrigramInvertedIndex::get_line_pos_at(const std::string & key) const {
    if (auto it = k_index_.find(key); it != k_index_.end()) {
        return it->second;
    }
    static const std::vector<size_t> empty;
    return empty;
}