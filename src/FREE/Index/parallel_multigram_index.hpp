#ifndef FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_
#define FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_

#include "multigram_index.hpp"

namespace free_index {

class ParallelMultigramIndex : public MultigramIndex {
 public:
    ParallelMultigramIndex() = delete;
    ParallelMultigramIndex(const ParallelMultigramIndex &&) = delete;
    ParallelMultigramIndex(const std::vector<std::string> & dataset, 
                   double sel_threshold, int num_threads)
      : MultigramIndex(dataset, sel_threshold) {
            size_t ds_size = dataset.size();
            thread_count_ = std::min(ds_size, size_t(std::max(4, num_threads)));
            k_line_range_.assign(thread_count_+1, 0);
            auto group_size = ds_size / thread_count_;
            for (size_t i = 1; i < thread_count_; i++) {
                k_line_range_[i] = k_line_range_[i-1] + group_size;
            }
            k_line_range_[thread_count_] = ds_size;
      }
    
    ~ParallelMultigramIndex() {}

    void build_index(int upper_n) override;

 protected:
    void select_grams(int upper_n) override;
    void fill_posting(int upper_n);
    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
    std::vector<size_t> k_line_range_;
        
 private:
    /**Select Grams Helpers**/
    void get_kgrams_not_indexed(
            std::unordered_map<std::string, long double> & kgrams,
            const std::unordered_set<std::string> & expand, size_t k);

    void get_uni_bigram(size_t idx,
        std::unordered_map<char, long double> & unigrams,
        std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams) ;

    void insert_kgram_into_index(
        const std::unordered_map<std::string, long double> & kgrams,
        std::unordered_set<std::string> & expand);
    /**Select Grams Helpers End**/
};

} // namespace free_index

#endif // FREE_INDEX_PARALLEL_MULTIGRAM_INDEX_HPP_