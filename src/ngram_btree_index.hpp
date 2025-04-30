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
    NGramBtreeIndex(const std::vector<std::string> & dataset,
        const std::vector<std::string> & queries) : NGramIndex(dataset, queries) {}
    ~NGramBtreeIndex() {}

    virtual void build_index(int upper_n) {}

    std::vector<std::string> find_all_keys(const std::string & line) const override;

    void print_index(bool size_only=false) const override {}

    void wirte_index_keys_to_file(const std::filesystem::path & out_path) const override;

    const std::vector<size_t> & get_line_pos_at(const std::string & key) const override;

    bool empty() const override { return k_index_keys_.empty(); }

    size_t get_num_keys() const override { return k_index_keys_.size(); }

    long long int get_bytes_used() const override;

    bool get_all_idxs(const std::string & reg, std::vector<size_t> & container) const {
        std::cerr << "should not be called" << std::endl;
        return true;
    };
    
 protected:
    // TODO: we can use abseil-btree also at https://abseil.io/about/design/btree
    /**Key is address of the multigram in k_index_keys_, 
     * value address of is a sorted (ascending) list of line indices**/
    btree::set<std::string> k_index_keys_;
    btree::map<std::string, std::vector<size_t>> k_index_;

    void find_all_keys_helper(
        const std::string & line,  std::vector<std::string> & found_keys) const override;
};

#endif // NGRAM_BTREE_INDEX_HPP_