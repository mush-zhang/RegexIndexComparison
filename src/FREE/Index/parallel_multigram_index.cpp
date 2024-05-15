#include <thread>
#include <future>
#include <sstream>
#include <cassert>
#include <cmath>

#include "parallel_multigram_index.hpp"

#ifdef NDEBUG
#define assert(x) (void(0))
#endif

// Use 'loc_' prefix to denote a variable that should be unique to a thread

template <typename T, class hash_T>
void free_index::ParallelMultigramIndex::add_or_inc_w_lock(
        std::map<T, atomic_ptr_t> & kgrams, 
        const T & key, std::unordered_set<T, hash_T> & loc_visited_kgrams) {
    if (!kgrams.contains(key)) {
        grams_mutex_.lock();
        if (!kgrams.contains(key))
            kgrams.insert({ key, atomic_ptr_t(new std::atomic_ulong(1))});
        else 
            (*(kgrams.at(key)))++;
        grams_mutex_.unlock();
    } else {
        grams_mutex_.lock_shared();
        (*(kgrams.at(key)))++;
        grams_mutex_.unlock_shared();
    }
    loc_visited_kgrams.insert(key);
}

void free_index::ParallelMultigramIndex::get_uni_bigram(size_t idx,
        std::map<char, atomic_ptr_t> & unigrams,
        std::map<std::pair<char, char>, atomic_ptr_t> & bigrams) {
    for (size_t i = k_line_range_[idx]; i < k_line_range_[idx+1]; i++) {
        auto line = k_dataset_[i];
        std::unordered_set<char> loc_visited_unigrams;
        std::unordered_set<std::pair<char, char>, hash_pair> loc_visited_bigrams;
        for (size_t i = 0; i + 1 < line.size(); i++) {
            // optimize: Naively get all chars
            char c1 = line.at(i);
            char c2 = line.at(i+1);
            if (loc_visited_unigrams.find(c1) == loc_visited_unigrams.end()) {
                add_or_inc_w_lock(unigrams, c1, loc_visited_unigrams);
            }
            if (loc_visited_unigrams.find(c2) == loc_visited_unigrams.end()) {
                add_or_inc_w_lock(unigrams, c2, loc_visited_unigrams);
            }
            auto curr_bigram = std::make_pair(c1, c2);
            if (loc_visited_bigrams.find(curr_bigram) == loc_visited_bigrams.end()) {
                add_or_inc_w_lock(bigrams, curr_bigram, loc_visited_bigrams);
            }
        }
    }
}

void free_index::ParallelMultigramIndex::insert_unigram_into_index(
        const std::map<char, atomic_ptr_t> & unigrams,
        std::map<char, atomic_ptr_t>::iterator s,
        std::map<char, atomic_ptr_t>::iterator d,
        std::vector<char> & loc_uni_expand,
        std::vector<char> & loc_index_keys) {
    for (; s != d; s++) {
        char c = s->first;
        if (*(s->second)/((double)k_dataset_size_) <= k_threshold_) {
            loc_index_keys.push_back(c);
        } else {
            loc_uni_expand.push_back(c);
        }
    }
}

void free_index::ParallelMultigramIndex::insert_bigram_into_index(
        const std::map<std::pair<char, char>, atomic_ptr_t> & bigrams,
        std::map<std::pair<char, char>, atomic_ptr_t>::iterator s,
        std::map<std::pair<char, char>, atomic_ptr_t>::iterator d,
        const std::unordered_set<char> & uni_expand,
        std::vector<std::pair<char, char>> & loc_bi_expand,
        std::vector<std::pair<char, char>> & loc_index_keys) {
    for (; s != d; s++) {
        auto p = s->first;
        // check if it is expand
        if (uni_expand.find(p.first) != uni_expand.end()) {
            if (*(s->second)/((double)k_dataset_size_) <= k_threshold_) {
                loc_index_keys.push_back(p);
            } else {
                loc_bi_expand.push_back(p);
            }
        }
    }
}

void free_index::ParallelMultigramIndex::get_kgrams_not_indexed(size_t idx,
        std::map<std::string, atomic_ptr_t> & kgrams,
        const std::unordered_set<std::string> & expand, size_t k) {
    // get all grams whose prefix in expand
    for (size_t i = k_line_range_[idx]; i < k_line_range_[idx+1]; i++) {
        auto line = k_dataset_[i];
        std::unordered_set<std::string> loc_visited_kgrams;
        for (size_t i = 0; i+k <= line.size(); i++) {
            auto curr_kgram = line.substr(i, k);
            // not seen in current line and in expand
            if (loc_visited_kgrams.find(curr_kgram) == loc_visited_kgrams.end() &&
                expand.find(line.substr(i, k-1)) != expand.end()) {
                add_or_inc_w_lock(kgrams, curr_kgram, loc_visited_kgrams);
            }
        }
    }
} 

void free_index::ParallelMultigramIndex::insert_kgram_into_index(
        const std::map<std::string, atomic_ptr_t> & kgrams,
        std::map<std::string, atomic_ptr_t>::iterator s,
        std::map<std::string, atomic_ptr_t>::iterator d,
        std::vector<std::string> & loc_expand,
        std::vector<std::string> & loc_index_keys) {
    // for each gram, if selectivity <= threshold, insert to index
    //    else insert to expand
    for (; s != d; s++) {
        auto str = s->first;
        if (*(s->second)/((double)k_dataset_size_) <= k_threshold_) {
            loc_index_keys.push_back(str);
        } else {
            loc_expand.push_back(str);
        }
    }
}

void free_index::ParallelMultigramIndex::select_grams(int upper_n) {
    std::unordered_set<std::string> expand; // stores useless prefix

    size_t posting_resv_size = std::ceil(k_dataset_size_*k_threshold_);

    std::map<char, atomic_ptr_t> unigrams;
    std::map<std::pair<char, char>, atomic_ptr_t> bigrams;
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
    std::vector<std::vector<char>> loc_uni_expands(thread_count_);
    std::vector<std::vector<char>> loc_index_keys_char(thread_count_);
    auto num_per_thread = std::ceil(unigrams.size() / ((double) thread_count_));
    for (size_t j = 0; j < thread_count_; j++) {
        loc_uni_expands[j].reserve(num_per_thread);
        loc_index_keys_char[j].reserve(num_per_thread);
    }
    int i = 0;
    auto uni_start_it = unigrams.begin();
    auto uni_end_it = uni_start_it;
    for (; i < thread_count_ && num_per_thread*(i+1) < unigrams.size(); i++) {
        std::advance(uni_end_it, num_per_thread);
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::insert_unigram_into_index, this,
                std::cref(unigrams), uni_start_it, uni_end_it, 
                std::ref(loc_uni_expands[i]), std::ref(loc_index_keys_char[i])
        ));
        uni_start_it = uni_end_it;
    }
    if (num_per_thread * i < unigrams.size()) {
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::insert_unigram_into_index, this,
                std::cref(unigrams), uni_start_it, unigrams.end(),
                std::ref(loc_uni_expands[i]), std::ref(loc_index_keys_char[i])
        ));
    }
    for (auto &th : threads) {
        th.join();
    }
    for (const auto & thread_local_vect : loc_uni_expands) {
        for (const auto & c : thread_local_vect) {
            uni_expand.insert(c);
        }
    }
    for (const auto & thread_local_vect : loc_index_keys_char) {
        for (const auto & c : thread_local_vect) {
            std::string curr_str = std::string(1, c);
            k_index_keys_.insert(curr_str);
            k_index_.insert({curr_str, std::vector<size_t>()});
        }
    }
    decltype(unigrams)().swap(unigrams);
    decltype(loc_uni_expands)().swap(loc_uni_expands);
    decltype(loc_index_keys_char)().swap(loc_index_keys_char);
    decltype(threads)().swap(threads);

    if (upper_n < 2) return;

    std::vector<std::vector<std::pair<char,char>>> loc_bi_expands(thread_count_);
    std::vector<std::vector<std::pair<char,char>>> loc_index_keys_pair(thread_count_);
    num_per_thread = std::ceil(bigrams.size() / ((double) thread_count_));
    i = 0;
    auto bi_start_it = bigrams.begin();
    auto bi_end_it = bi_start_it;
    for (; i < thread_count_ && num_per_thread*(i+1) < bigrams.size(); i++) {
        std::advance(bi_end_it, num_per_thread);
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::insert_bigram_into_index, this,
                std::cref(bigrams), bi_start_it, bi_end_it,
                std::cref(uni_expand),
                std::ref(loc_bi_expands[i]), 
                std::ref(loc_index_keys_pair[i])
        ));
        bi_start_it = bi_end_it;
    }
    if (num_per_thread * i < bigrams.size()) {
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::insert_bigram_into_index, this,
                std::cref(bigrams), bi_start_it, bigrams.end(),
                std::cref(uni_expand),
                std::ref(loc_bi_expands[i]), 
                std::ref(loc_index_keys_pair[i])
        ));
    }
    for (auto &th : threads) {
        th.join();
    }
    for (const auto & thread_local_vect : loc_bi_expands) {
        for (const auto & p : thread_local_vect) {
            std::string curr_str{p.first, p.second};
            expand.insert(curr_str);
        }
    }
    for (const auto & thread_local_vect : loc_index_keys_pair) {
        for (const auto & p : thread_local_vect) {
            std::string curr_str{p.first, p.second};
            k_index_keys_.insert(curr_str);
            k_index_.insert({curr_str, std::vector<size_t>()});
        }
    }
    decltype(bigrams)().swap(bigrams);
    decltype(loc_bi_expands)().swap(loc_bi_expands);
    decltype(loc_index_keys_pair)().swap(loc_index_keys_pair);
    decltype(threads)().swap(threads);

    int k = 3;
    while (!expand.empty() && k <= upper_n) {
        // get all k-grams whose prefix not in index already
        std::map<std::string, atomic_ptr_t> curr_kgrams = {};

        for (int i = 0; i < thread_count_; i++) {
            threads.push_back(std::thread(
                &free_index::ParallelMultigramIndex::get_kgrams_not_indexed, this,
                    i, std::ref(curr_kgrams), std::cref(expand), k
            ));
        }
        for (auto &th : threads) {
            th.join();
        }
        decltype(threads)().swap(threads);
        // Clear the expand for current k
        decltype(expand)().swap(expand);

        std::vector<std::vector<std::string>> loc_expands(thread_count_);
        std::vector<std::vector<std::string>> loc_index_keys(thread_count_);
        num_per_thread = std::ceil(curr_kgrams.size() / ((double) thread_count_));
        i = 0;
        auto start_it = curr_kgrams.begin();
        auto end_it = start_it;
        for (; i < thread_count_ && num_per_thread*(i+1) < curr_kgrams.size(); i++) {
            std::advance(end_it, num_per_thread);
            threads.push_back(std::thread(
                &free_index::ParallelMultigramIndex::insert_kgram_into_index, this,
                    std::cref(curr_kgrams), start_it, end_it, 
                    std::ref(loc_expands[i]), 
                    std::ref(loc_index_keys[i])
            ));
            start_it = end_it;
        }
        if (num_per_thread * i < curr_kgrams.size()) {
            threads.push_back(std::thread(
                &free_index::ParallelMultigramIndex::insert_kgram_into_index, this,
                    std::cref(curr_kgrams), start_it, curr_kgrams.end(), 
                    std::ref(loc_expands[i]), 
                    std::ref(loc_index_keys[i])
            ));
        }
        for (auto &th : threads) {
            th.join();
        }
        for (const auto & thread_local_vect : loc_expands) {
            for (const auto & s : thread_local_vect) {
                expand.insert(s);
            }
        }
        for (const auto & thread_local_vect : loc_index_keys) {
            for (const auto & s : thread_local_vect) {
                k_index_keys_.insert(s);
                k_index_.insert({s, std::vector<size_t>()});
            }
        }
        decltype(curr_kgrams)().swap(curr_kgrams);
        decltype(loc_expands)().swap(loc_expands);
        decltype(loc_index_keys)().swap(loc_index_keys);
        decltype(threads)().swap(threads);
        k++;
    }
}

void free_index::ParallelMultigramIndex::kgrams_in_line(int upper_n, size_t idx,
        std::unordered_map<std::string, std::vector<size_t>> & local_idx) {
    for (size_t i = k_line_range_[idx]; i < k_line_range_[idx+1]; i++) {
        auto line = k_dataset_[i];
        for (auto pos = 0; pos < line.size(); pos++) {
            for (auto k = 1; k <= upper_n && k + pos <= line.size(); k++) {
                const auto curr_substr = line.substr(pos, k);
                if (k_index_keys_.find(curr_substr) != k_index_keys_.end() &&
                    (local_idx[curr_substr].size() == 0 ||
                     local_idx[curr_substr].back() < i)
                    ) {
                    local_idx[curr_substr].push_back(i);
                    // any longer ones will not be in the index; increment to next pos
                    break;
                }
            }
        }
    }
}

void free_index::ParallelMultigramIndex::merge_lists(
        std::set<std::string>::const_iterator s_o, std::set<std::string>::const_iterator d_o,
        const std::vector<std::unordered_map<std::string, std::vector<size_t>>> & loc_idxs) {
    for (std::set<std::string>::const_iterator s = s_o; s != d_o; ++s) {
        auto key = *s;
        for (auto & sub_map : loc_idxs) {
            if (sub_map.contains(key))
                k_index_[key].insert(k_index_[key].end(), sub_map.at(key).cbegin(), sub_map.at(key).cend());            
        }
    }
}

void free_index::ParallelMultigramIndex::fill_posting(int upper_n) {
    std::vector<std::thread> threads;
    std::vector<std::unordered_map<std::string, std::vector<size_t>>> loc_idxs(thread_count_);
    for (int i = 0; i < thread_count_; i++) {
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::kgrams_in_line, this,
                upper_n, i, std::ref(loc_idxs[i])
        ));
    }
    for (auto &th : threads) {
        th.join();
    }
    decltype(threads)().swap(threads);

    auto num_per_thread = std::ceil(k_index_keys_.size() / ((double) thread_count_));
    int i = 0;
    auto start_it = k_index_keys_.cbegin();
    auto end_it = start_it;
    for (; i < thread_count_ && num_per_thread*(i+1) <= k_index_keys_.size(); i++) {
        std::advance(end_it, num_per_thread);
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::merge_lists, this,
                start_it, end_it, std::cref(loc_idxs)
        ));
        start_it = end_it;
    }
    if (num_per_thread * i < k_index_keys_.size()) {
        threads.push_back(std::thread(
            &free_index::ParallelMultigramIndex::merge_lists, this,
                start_it, k_index_keys_.cend(), std::cref(loc_idxs)
        ));
    }
    for (auto &th : threads) {
        th.join();
    }

    decltype(loc_idxs)().swap(loc_idxs);
    decltype(threads)().swap(threads);
}

template 
void free_index::ParallelMultigramIndex::add_or_inc_w_lock<char, std::hash<char>>(
        std::map<char, atomic_ptr_t> & kgrams, 
        const char & key, std::unordered_set<char, std::hash<char>> & loc_visited_kgrams);

template 
void free_index::ParallelMultigramIndex::add_or_inc_w_lock<std::pair<char,char>, hash_pair>(
        std::map<std::pair<char,char>, atomic_ptr_t> & kgrams, 
        const std::pair<char,char> & key, 
        std::unordered_set<std::pair<char,char>, hash_pair> & loc_visited_kgrams);

template 
void free_index::ParallelMultigramIndex::add_or_inc_w_lock<std::string, std::hash<std::string>>(
        std::map<std::string, atomic_ptr_t> & kgrams, 
        const std::string & key, 
        std::unordered_set<std::string, std::hash<std::string>> & loc_visited_kgrams);    