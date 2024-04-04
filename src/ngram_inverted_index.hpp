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
    ~NGramInvertedIndex() {}

    virtual void build_index(int upper_k) {}

    std::vector<std::string> find_all_indexed(const std::string & line) override;

    void print_index() override;
    
    const std::vector<unsigned int> & get_line_pos_at(const std::string & key) override;

 protected:
    /**Key is multigram, value is a sorted (ascending) list of line indices**/
    std::unordered_map<std::string, std::vector<unsigned int>> k_index_;
    std::set<std::string> k_index_keys_;
};

#endif // NGRAM_INVERTED_INDEX_HPP_