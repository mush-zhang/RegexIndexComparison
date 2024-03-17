#include "hash_pair.hpp"

#ifndef FREE_INDEX_MULTIGRAM_INDEX_HPP_
#define FREE_INDEX_MULTIGRAM_INDEX_HPP_

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>

namespace free_index {
static const std::vector<long> k_empty_pos_list_;

class MultigramIndex {
 public:
    MultigramIndex() = delete;
    MultigramIndex(const std::vector<std::string> &&) = delete;
    MultigramIndex(const std::vector<std::string> & dataset, double sel_threshold)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), k_threshold_(sel_threshold) {}
    
    ~MultigramIndex() {}

    virtual void build_index(int upper_k);

    // return all substrings of the given string that are keys in the index
    std::vector<std::string> find_all_indexed(const std::string & line);

    void print_index();
    
    const std::vector<long> & get_line_pos_at(std::string key) const { 
        if (auto it = k_index_.find(key); it != k_index_.end()) {
            return it->second;
        }
        return k_empty_pos_list_;
    }

 protected:
    void select_grams(int upper_k);
    void fill_posting(int upper_k);
    std::set<std::string> k_index_keys_;
    
 private:
    // the index structure should be stored here
    const std::vector<std::string> &k_dataset_;
    const long double k_dataset_size_;
    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
    std::unordered_map<std::string, std::vector<long>> k_index_;

    /**Select Grams Helpers**/
    void get_kgrams_not_indexed(
            std::unordered_map<std::string, long double> & kgrams,
            const std::unordered_set<std::string> & expand, size_t k);
    void get_uni_bigram(
        std::unordered_map<char, long double> & unigrams,
        std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams);

    void insert_kgram_into_index(
        const std::unordered_map<std::string, long double> & kgrams,
        std::unordered_set<std::string> & expand);
    void insert_uni_bigram_into_index(
        const std::unordered_map<char, long double> & unigrams,
        const std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams,
        std::unordered_set<std::string> & expand);
    /**Select Grams Helpers End**/
};

} // namespace free_index

#endif // FREE_INDEX_MULTIGRAM_INDEX_HPP_