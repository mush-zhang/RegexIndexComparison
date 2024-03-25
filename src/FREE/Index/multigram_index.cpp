#include "multigram_index.hpp"
#include <iostream>
#include <chrono>

/**-----------------------------Helpers Start----------------------------------**/

using gram_count_map = std::unordered_map<std::string, long double>;
using gram_set = std::unordered_set<std::string>;

template <typename T, class hash_T>
static void insert_or_increment(std::unordered_map<T, long double, hash_T> & kgrams, 
                                T & key,
                                std::unordered_set<T, hash_T> & visited_kgrams) {
    auto [it, insert_success] = kgrams.insert({ key, 1 });
    if (!insert_success) {
        // Element was already present; increment the value at the key.
        ++(it->second);
    }
    visited_kgrams.insert(key);
}

/**-----------------------------Helpers End----------------------------------**/

// Algorithm 3.1 Multigram Index
void free_index::MultigramIndex::build_index(int upper_k) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_k);
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << elapsed << " s" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    fill_posting(upper_k);
    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}

void free_index::MultigramIndex::fill_posting(int upper_k) {
    for (auto i = 0; i < k_dataset_size_; i++) {
        auto line = k_dataset_[i];
        for (auto pos = 0; pos < line.size(); pos++) {
            for (auto k = 1; k <= upper_k && k + pos <= line.size(); k++) {
                const auto curr_substr = line.substr(pos, k);
                if (k_index_keys_.find(curr_substr) != k_index_keys_.end() &&
                    (k_index_[curr_substr].size() == 0 ||
                     k_index_[curr_substr].back() < i)
                    ) {
                    k_index_[curr_substr].push_back(i);
                    // any longer ones will not be in the index; increment to next pos
                    break;
                }
            }
        }
    }
}


void free_index::MultigramIndex::select_grams(int upper_k) {
    gram_set expand; // stores useless prefix

    // Opimization page 6
    // In the first iteration, find useless grams for both k=1 and k=2
    std::unordered_map<char, long double> unigrams;
    std::unordered_map<std::pair<char, char>, long double, hash_pair> bigrams;
    get_uni_bigram(unigrams, bigrams);
    insert_uni_bigram_into_index(unigrams, bigrams, expand);
    decltype(unigrams)().swap(unigrams);
    decltype(bigrams)().swap(bigrams);

    int k = 3;
    while (!expand.empty() && k <= upper_k) {
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

// Algorithm 3.1 
//    line [3]: get all k-grams in database whose (k-1)-prefix in expand
// We also record M(key), the number of data units which contains key
//    for selectivity calculation defined in Definition 3.1
void free_index::MultigramIndex::get_kgrams_not_indexed(
        std::unordered_map<std::string, long double> & kgrams,
        const std::unordered_set<std::string> & expand, size_t k) {
    // get all grams whose prefix in expand
    for (const auto & line : k_dataset_) {
        gram_set visited_kgrams;
        for (size_t i = 0; i+k <= line.size(); i++) {
            auto curr_kgram = line.substr(i, k);
            // not seen in current line and in expand
            if (visited_kgrams.find(curr_kgram) == visited_kgrams.end() &&
                expand.find(line.substr(i, k-1)) != expand.end()) {
                insert_or_increment(kgrams, curr_kgram, visited_kgrams);
            }
        }
    }
} 

void free_index::MultigramIndex::get_uni_bigram(
        std::unordered_map<char, long double> & unigrams,
        std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams) {
    for (const auto & line : k_dataset_) {
        std::unordered_set<char> visited_unigrams;
        std::unordered_set<std::pair<char, char>, hash_pair> visited_bigrams;
        for (size_t i = 0; i + 1 < line.size(); i++) {
            // optimize: Naively get all chars
            char c1 = line.at(i);
            char c2 = line.at(i+1);
            if (visited_unigrams.find(c1) == visited_unigrams.end()) {
                insert_or_increment(unigrams, c1, visited_unigrams);
            }
            if (visited_unigrams.find(c2) == visited_unigrams.end()) {
                insert_or_increment(unigrams, c2, visited_unigrams);
            }
            auto curr_bigram = std::make_pair(c1, c2);
            if (visited_bigrams.find(curr_bigram) == visited_bigrams.end()) {
                insert_or_increment(bigrams, curr_bigram, visited_bigrams);
            }
        }
    }
}

void free_index::MultigramIndex::insert_uni_bigram_into_index(
        const std::unordered_map<char, long double> & unigrams,
        const std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams,
        std::unordered_set<std::string> & expand) {
    // for each gram, if selectivity <= threshold, insert to index
    //    else insert to expand
    std::unordered_set<char> uni_expand;
    for (const auto & [c, c_count] : unigrams) {
        if (c_count/k_dataset_size_ <= k_threshold_) {
            k_index_keys_.insert(std::string(1, c));
        } else {
            uni_expand.insert(c);
        }
    }
    for (const auto & [p, p_count] : bigrams) {
        // check if it is expand
        if (uni_expand.find(p.first) != uni_expand.end()) {
            std::string curr_str{p.first, p.second};
            if (p_count/k_dataset_size_ <= k_threshold_) {
                k_index_keys_.insert(curr_str);
            } else {
                expand.insert(curr_str);
            }
        }
    }
}

void free_index::MultigramIndex::insert_kgram_into_index(
        const std::unordered_map<std::string, long double> & kgrams,
        std::unordered_set<std::string> & expand) {
    // for each gram, if selectivity <= threshold, insert to index
    //    else insert to expand
    for (const auto & [s, s_count] : kgrams) {
        if (s_count/k_dataset_size_ <= k_threshold_) {
            k_index_keys_.insert(s);
        } else {
            expand.insert(s);
        }
    }
}

