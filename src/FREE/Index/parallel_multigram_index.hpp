#ifndef FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_
#define FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_

#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <shared_mutex>

#include "multigram_index.hpp"

using atomic_ptr_t = std::shared_ptr<std::atomic_ulong>;

namespace free_index {

class ParallelMultigramIndex : public MultigramIndex {
 public:
    ParallelMultigramIndex() = delete;
    ParallelMultigramIndex(const ParallelMultigramIndex &&) = delete;
    ParallelMultigramIndex(const std::vector<std::string> & dataset, 
                   double sel_threshold, int num_threads)
      : MultigramIndex(dataset, sel_threshold) {
            k_tag_ = "-parallel";
            size_t ds_size = dataset.size();
            thread_count_ = std::min(ds_size, size_t(std::max(2, num_threads)));
            k_line_range_.assign(thread_count_+1, 0);
            auto group_size = ds_size / thread_count_;
            for (size_t i = 1; i < thread_count_; i++) {
                k_line_range_[i] = k_line_range_[i-1] + group_size;
            }
            k_line_range_[thread_count_] = ds_size;
        }
    
    ~ParallelMultigramIndex() {}

    void build_index(int upper_n) override { MultigramIndex::build_index(upper_n); }

 protected:
    void select_grams(int upper_n) override;
    void fill_posting(int upper_n) override;

    std::vector<size_t> k_line_range_;
    std::shared_mutex grams_mutex_;

 private:
    /**Select Grams Helpers**/
    template <typename T, class hash_T>
    void add_or_inc_w_lock(
        std::map<T, atomic_ptr_t> & kgrams, 
        const T & key, std::unordered_set<T, hash_T> & loc_visited_kgrams);
    
    void get_uni_bigram(size_t idx,
        std::map<char, atomic_ptr_t> & unigrams,
        std::map<std::pair<char, char>, atomic_ptr_t> & bigrams);
    
    void insert_unigram_into_index(const std::map<char, atomic_ptr_t> & unigrams,
        std::map<char, atomic_ptr_t>::iterator s,
        std::map<char, atomic_ptr_t>::iterator d,
        std::vector<char> & loc_uni_expand,
        std::vector<char> & loc_index_keys);
    
    void insert_bigram_into_index(
        const std::map<std::pair<char, char>, atomic_ptr_t> & bigrams,
        std::map<std::pair<char, char>, atomic_ptr_t>::iterator s,
        std::map<std::pair<char, char>, atomic_ptr_t>::iterator d,
        const std::unordered_set<char> & uni_expand,
        std::vector<std::pair<char, char>> & loc_bi_expand,
        std::vector<std::pair<char, char>> & loc_index_keys);

    void get_kgrams_not_indexed(size_t idx,
        std::map<std::string, atomic_ptr_t> & kgrams,
        const std::unordered_set<std::string> & expand, size_t k);

    void insert_kgram_into_index(
        const std::map<std::string, atomic_ptr_t> & kgrams,
        std::map<std::string, atomic_ptr_t>::iterator s,
        std::map<std::string, atomic_ptr_t>::iterator d,
        std::vector<std::string> & loc_expand,
        std::vector<std::string> & loc_index_keys);
    /**Select Grams Helpers End**/

    void kgrams_in_line(int upper_n, size_t idx, 
        std::unordered_map<std::string, std::vector<size_t>> & local_idx);
    
    void merge_lists(
        std::set<std::string>::const_iterator s_o, std::set<std::string>::const_iterator d_o,
        const std::vector<std::unordered_map<std::string, std::vector<size_t>>> & loc_idxs);
};

} // namespace free_index

#endif // FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_