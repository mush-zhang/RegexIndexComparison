#ifndef NGRAM_INVERTED_INDEX_HPP_
#define NGRAM_INVERTED_INDEX_HPP_

#include <unordered_map>
#include "ngram_index.hpp"

class NGramInvertedIndex : public NGramIndex {
 public:
    NGramInvertedIndex() = delete;
    NGramInvertedIndex(const NGramInvertedIndex &&) = delete;
    NGramInvertedIndex(const std::vector<std::string> & dataset) : NGramIndex(dataset) {
        // https://www.geeksforgeeks.org/how-to-use-unordered_map-efficiently-in-c/
        k_index_.reserve(1024); // RESERVING SPACE BEFOREHAND
        k_index_.max_load_factor(0.25); // DECREASING MAX_LOAD_FACTOR
    }
    
    NGramInvertedIndex(const std::vector<std::string> & dataset, 
            const std::vector<std::string> & queries) : NGramIndex(dataset, queries) {
        // https://www.geeksforgeeks.org/how-to-use-unordered_map-efficiently-in-c/
        k_index_.reserve(1024); // RESERVING SPACE BEFOREHAND
        k_index_.max_load_factor(0.25); // DECREASING MAX_LOAD_FACTOR
    }

    ~NGramInvertedIndex() {}

    virtual void build_index(int upper_n) {}

    std::vector<std::string> find_all_indexed(const std::string & reg) const override;

    void print_index(bool size_only=false) const override;
    
    const std::vector<size_t> & get_line_pos_at(const std::string & key) const override;

    bool empty() const override { return k_index_keys_.empty(); }

    size_t get_num_keys() const override { return k_index_keys_.size(); }

    size_t get_bytes_used() const override;

 protected:
    /**Key is multigram, value is a sorted (ascending) list of line indices**/
    std::unordered_map<std::string, std::vector<size_t>> k_index_;
    std::set<std::string> k_index_keys_;

    void find_all_indexed_helper(
        const std::string & line,  std::vector<std::string> & found_keys) const override;
};

#endif // NGRAM_INVERTED_INDEX_HPP_