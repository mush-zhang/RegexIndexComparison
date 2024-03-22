// #include "hash_pair.hpp"

#ifndef BEST_INDEX_NAIVE_HPP_
#define BEST_INDEX_NAIVE_HPP_

#include <vector>
#include <string>
#include <set>
#include <unordered_map>

namespace best_index {
static const std::vector<long> k_empty_pos_list_;

class NaiveIndex {
 public:
    NaiveIndex() = delete;
    NaiveIndex(const std::vector<std::string> &&) = delete;
    NaiveIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold, int max_num_keys=-1)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), 
        k_queries_(queries), k_queries_size_(queries.size()),
        k_threshold_(sel_threshold), k_max_num_keys_(max_num_keys) {}
    
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
    const long double k_queries_size_;
    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
    const int k_max_num_keys_;
    const std::vector<std::string> & k_queries_;
    const std::vector<std::string> & k_dataset_;

    /**Key is multigram, value is a sorted (ascending) list of line indices**/
    std::unordered_map<std::string, std::vector<long>> k_index_;

    std::vector<std::string> find_all_indexed(const std::string & line);

    /** Helpers **/
    std::set<std::string> candidate_gram_set_gen();


};

} // namespace best_index

#endif // BEST_INDEX_NAIVE_HPP_