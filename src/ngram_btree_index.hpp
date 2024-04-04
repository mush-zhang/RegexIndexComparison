#ifndef NGRAM_BTREE_INDEX_HPP_
#define NGRAM_BTREE_INDEX_HPP_

#include "utils/cpp-btree/btree/map.h"
#include "utils/cpp-btree/btree/set.h"
#include "ngram_index.hpp"

class NGramBtreeIndex : public NGramIndex {
 public:
    NGramBtreeIndex() = delete;
    NGramBtreeIndex(const NGramBtreeIndex &&) = delete;
    NGramBtreeIndex(const std::vector<std::string> & dataset) : NGramIndex(dataset) {}
    ~NGramBtreeIndex() {}

    virtual void build_index(int upper_k) {}

    std::vector<std::string> find_all_indexed(const std::string & line) override;

    void print_index() override;
    
    const std::vector<unsigned int> & get_line_pos_at(const std::string & key) override;

 protected:
    /**Key is address of the multigram in k_index_keys_, 
     * value address of is a sorted (ascending) list of line indices**/
    btree::set<std::string> k_index_keys_;
    btree::map<std::string, std::vector<unsigned int>> k_index_;
};

#endif // NGRAM_BTREE_INDEX_HPP_