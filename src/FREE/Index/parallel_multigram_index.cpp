#include <thread>
#include <future>
#include <sstream>
#include <atomic>

#include "parallel_multigram_index.hpp"

using gram_count_map = std::unordered_map<std::string, long double>;
using gram_set = std::unordered_set<std::string>;


void free_index::ParallelMultigramIndex::uni_bi_job(std::set<std::string> & idx_keys,
        const std::map<char, std::atomic_ulong> unigrams,
        const std::map<std::pair<char, char>, std::atomic_ulong, hash_pair> bigrams) {
    get_uni_bigram(unigrams, bigrams);
    insert_uni_bigram_into_index(unigrams, bigrams, expand, idx_keys);
    decltype(unigrams)().swap(unigrams);
    decltype(bigrams)().swap(bigrams);
}

void free_index::ParallelMultigramIndex::insert_uni_into_index(
        const std::unordered_map<char, long double> & unigrams,
        const std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams,
        std::unordered_set<std::string> & expand,
        std::set<std::string> & index_keys) {
    // for each gram, if selectivity <= threshold, insert to index
    //    else insert to expand
    std::unordered_set<char> uni_expand;
    for (const auto & [c, c_count] : unigrams) {
        if (c_count/((double)k_dataset_size_) <= k_threshold_) {
            index_keys.insert(std::string(1, c));
        } else {
            uni_expand.insert(c);
        }
    }
    for (const auto & [p, p_count] : bigrams) {
        // check if it is expand
        if (uni_expand.find(p.first) != uni_expand.end()) {
            std::string curr_str{p.first, p.second};
            if (p_count/((double)k_dataset_size_) <= k_threshold_) {
                index_keys.insert(curr_str);
            } else {
                expand.insert(curr_str);
            }
        }
    }
}

void free_index::ParallelMultigramIndex::uni_job(
        const std::map<char, std::atomic_ulong> & unigrams,
        std::vector<char> & uni_expand,
        std::vector<char> & index_keys) {

    for (const auto & [c, c_count] : unigrams) {
        if (c_count/((double)k_dataset_size_) <= k_threshold_) {
            index_keys.push_back(c);
        } else {
            uni_expand.push_back(c);
        }
    }
}

void free_index::ParallelMultigramIndex::bi_job(
        const std::map<std::pair<char, char>, std::atomic_ulong> & bigrams,
        const std::unordered_set<char> & uni_expand,
        std::vector<std::pair<char, char>> & bi_expand,
        std::vector<std::pair<char, char>> & index_keys) {

    for (const auto & [p, p_count] : bigrams) {
        // check if it is expand
        if (uni_expand.find(p.first) != uni_expand.end()) {
            if (p_count/((double)k_dataset_size_) <= k_threshold_) {
                index_keys.insert(p);
            } else {
                bi_expand.insert(p);
            }
        }
    }
}

void free_index::ParallelMultigramIndex::select_grams(int upper_n) {
    gram_set expand; // stores useless prefix

    std::map<char, std::atomic_ulong> unigrams;
    std::map<std::pair<char, char>, std::atomic_ulong, hash_pair> bigrams;
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count_; i++) {
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::get_uni_bigram, this,
                i, std::ref(unigrams), std::ref(bigrams)
        ));
    }
    for (auto &th : threads) {
        th.join();
    }
    decltype(threads)().swap(threads);

    std::unordered_set<char> uni_expand;
    std::vector<std::vector<char>> local_uni_expands(thread_count_);
    std::vector<std::vector<char>> local_index_keys_char(thread_count_);
    auto num_per_thread = std::max(1, unigrams.size() / thread_count_);
    int i = 0;
    for (; i < thread_count_ && num_per_thread*(i+1) < unigrams.size(); i++) {
        auto curr_uni_map = std::map{
            std::make_move_iterator(unigrams.begin() + num_per_thread*i),
            std::make_move_iterator(unigrams.begin() + num_per_thread*(i+1))
        };
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::uni_job, this,
                std::cref(curr_uni_map), std::ref(local_uni_expands[i]), 
                std::ref(local_index_keys_char[i])
        ));
    }
    auto curr_uni_map = std::map{
        std::make_move_iterator(unigrams.begin() + num_per_thread*i),
        std::make_move_iterator(unigrams.begin() + unigrams.size())
    };
    threads.push_back(std::thread(
        &free_index::ParallelMultigramIndex::uni_job, this,
            std::cref(curr_uni_map), std::ref(local_uni_expands[i]), 
            std::ref(local_index_keys_char[i])
    ));
    for (auto &th : threads) {
        th.join();
    }
    for (const auto & thread_local_vect : local_uni_expands) {
        for (const auto & c : thread_local_vect) {
            uni_expand.insert(c);
        }
    }
    for (const auto & thread_local_vect : local_index_keys_char) {
        for (const auto & c : thread_local_vect) {
            k_index_keys_.insert(std::string(1, c));
        }
    }
    decltype(unigrams)().swap(unigrams);
    decltype(local_uni_expands)().swap(local_uni_expands);
    decltype(local_index_keys_char)().swap(local_index_keys_char);
    decltype(threads)().swap(threads);

    std::vector<std::vector<std::pair<char,char>>> local_bi_expands(thread_count_);
    std::vector<std::vector<std::pair<char,char>>> local_index_keys_pair(thread_count_);
    auto num_per_thread = std::max(1, bigrams.size() / thread_count_);
    int i = 0;
    for (; i < thread_count_ && num_per_thread*(i+1) < bigrams.size(); i++) {
        auto curr_bi_map = std::map{
            std::make_move_iterator(bigrams.begin() + num_per_thread*i),
            std::make_move_iterator(bigrams.begin() + num_per_thread*(i+1))
        };
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::bi_job, this,
                std::cref(curr_bi_map), std::cref(uni_expand),
                std::ref(local_bi_expands[i]), 
                std::ref(local_index_keys_pair[i])
        ));
    }
    auto curr_bi_map = std::map{
        std::make_move_iterator(bigrams.begin() + num_per_thread*i),
        std::make_move_iterator(bigrams.begin() + bigrams.size())
    };
    threads.push_back(std::thread(
        &free_index::ParallelMultigramIndex::bi_job, this,
            std::cref(curr_bi_map), std::cref(uni_expand),
            std::ref(local_bi_expands[i]), 
            std::ref(local_index_keys_pair[i])
    ));
    for (auto &th : threads) {
        th.join();
    }
    for (const auto & thread_local_vect : local_bi_expands) {
        for (const auto & c : thread_local_vect) {
            uni_expand.insert(c);
        }
    }
    for (const auto & thread_local_vect : local_index_keys_pair) {
        for (const auto & p : thread_local_vect) {
            std::string curr_str{p.first, p.second};
            k_index_keys_.insert(curr_str);
        }
    }
    decltype(bigrams)().swap(bigrams);
    decltype(threads)().swap(threads);

    int k = 3;
    while (!expand.empty() && k <= upper_n) {
        // get all k-grams whose prefix not in index already
        std::unordered_map<std::string, long double> curr_kgrams = {};
        get_kgrams_not_indexed(curr_kgrams, expand, k);

        // Clear the expand for current k
        decltype(expand)().swap(expand);

        // insert into index or expand
        insert_kgram_into_index(curr_kgrams, expand);

        k++;
    }
}

void free_index::MultigramIndex::get_uni_bigram(size_t idx,
        std::unordered_map<char, std::atomic_ulong> & unigrams,
        std::unordered_map<std::pair<char, char>, std::atomic_ulong, hash_pair> & bigrams) {
    for (size_t i = k_line_range_[idx]; i < k_line_range_[idx+1]; i++) {
        get_uni_bigram_helper(k_dataset_[i], unigrams, bigrams);
    }
}