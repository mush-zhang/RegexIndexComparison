// #include "hash_pair.hpp"

#ifndef BEST_INDEX_NAIVE_HPP_
#define BEST_INDEX_NAIVE_HPP_

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include "../../utils/trie.hpp"

extern "C" {
    // rax *raxNew(void);
    // int raxTryInsert(rax *rax, unsigned char *s, size_t len, void *data, void **old);
    #include "../../utils/rax/rax.h"
    #include "../../utils/rax/rc4rand.h"
};

namespace best_index {
static const std::vector<long> k_empty_pos_list_;

class NaiveIndex {
 public:
    NaiveIndex() = delete;
    NaiveIndex(const std::vector<std::string> &&) = delete;
    NaiveIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), 
        k_queries_(queries), k_threshold_(sel_threshold) {
            gram_tree_ = raxNew();
        }
    
    ~NaiveIndex() { if (gram_tree_) delete gram_tree_; }

    virtual void build_index(int upper_k);

    // // return all substrings of the given string that are keys in the index
    // std::vector<std::string> find_all_indexed(const std::string & line);

    void print_index();
    
    // const std::vector<long> & get_line_pos_at(const std::string & key) const { 
    //     if (auto it = k_index_.find(key); it != k_index_.end()) {
    //         return it->second;
    //     }
    //     return k_empty_pos_list_;
    // }

    // const std::vector<std::string> & get_dataset() const {
    //     return k_dataset_;
    // }

 protected:
    void select_grams(int upper_k);
    // void fill_posting(int upper_k);
    std::set<std::string> k_index_keys_;
    
 private:

    const long double k_dataset_size_;
    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
    const std::vector<std::string> & k_queries_;
    // the index structure should be stored here
    const std::vector<std::string> & k_dataset_;
    // trie_type gram_tree_;
    rax *gram_tree_ = nullptr;

    /**Key is multigram, value is a sorted (ascending) list of line indices**/
    std::unordered_map<std::string, std::vector<long>> k_index_;

    std::unordered_set<std::string> candidate_gram_set_gen();
    std::vector<std::string> find_all_indexed(const std::string & line);
    int insert_gram(const std::string & l);
    std::vector<std::string> generate_path_labels();

    // /**Select Grams Helpers**/
    // // TODO: return types
    // // Calculate the gram query cover - 
    // //   number of lines enabled/eliminated by the gram for a query
    // //   essentially #I{g \in q} * #r_{g \not in r}
    // gram_cover(const std::string & gram, const std::string & line);
    // // Calculate the cover - number of lines enabled/eliminated by the gram
    // //   essentially #q_{g \in q} * #r_{g \not in r}
    // gram_cover(const std::string & gram);
    // // Calculate the index cover - number of lines enabled/eliminated by the set of grams
    // //   essentially calculated incrementally:
    // //   from the one with largest gram cover, 
    // //   recalculate incremental gram covers as we add each gram in the index/set
    // index_cover(const std::set<std::string> & index);

    /**Select Grams Helpers End**/
};

} // namespace best_index

#endif // BEST_INDEX_NAIVE_HPP_