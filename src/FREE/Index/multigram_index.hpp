#ifndef FREE_INDEX_MULTIGRAM_INDEX_HPP_
#define FREE_INDEX_MULTIGRAM_INDEX_HPP_

#include <unordered_set>
#include "../../utils/hash_pair.hpp"
#include "../../ngram_inverted_index.hpp"

namespace free_index {

class MultigramIndex : public NGramInvertedIndex {
 public:
    MultigramIndex() = delete;
    MultigramIndex(const MultigramIndex &&) = delete;
    MultigramIndex(const std::vector<std::string> & dataset, double sel_threshold)
      : NGramInvertedIndex(dataset), k_threshold_(sel_threshold) {}
    
    ~MultigramIndex() {}

    void build_index(int upper_k) override;

    // // return all substrings of the given string that are keys in the index
    // std::vector<std::string> find_all_indexed(const std::string & line);

    // void print_index();
    
    // const std::vector<long> & get_line_pos_at(const std::string & key) const { 
    //     if (auto it = k_index_.find(key); it != k_index_.end()) {
    //         return it->second;
    //     }
    //     return k_empty_pos_list_;
    // }

 protected:
    void select_grams(int upper_k) override;
    void fill_posting(int upper_k);
    
 private:
    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;

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