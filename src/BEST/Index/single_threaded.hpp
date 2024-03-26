#ifndef BEST_INDEX_SINGLE_THREADED_HPP_
#define BEST_INDEX_SINGLE_THREADED_HPP_

#include "../../ngram_inverted_index.hpp"

namespace best_index {

/**
 * Note: the orginal paper assumes b+tree for size constraint calculation.
 *       Yet it does not actually build the index and measure query performance.
 *       It uses the ARE (false positives related measurement) to measure the.
 *       effectiveness of index
 */
class SingleThreadedIndex  : public NGramInvertedIndex {
 public:
    SingleThreadedIndex() = delete;
    SingleThreadedIndex(const SingleThreadedIndex &&) = delete;
    SingleThreadedIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold, int max_num_keys=-1)
      : NGramInvertedIndex(dataset), 
        k_queries_(queries), k_queries_size_(queries.size()),
        k_threshold_(sel_threshold), k_max_num_keys_(max_num_keys) {}
    
    ~SingleThreadedIndex() {}

    // No constraint on size of gram in BEST, use build_index();
    void build_index(int upper_k=-1) override;

 protected:
    void select_grams(int upper_k=-1) override;
    
 private:
    long double k_queries_size_;
    std::vector<std::string> & k_queries_;

    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
    const int k_max_num_keys_;

    /** Helpers **/
    std::vector<std::string> candidate_gram_set_gen(
      std::vector<std::vector<std::string>> & query_literals);


};

} // namespace best_index

#endif // BEST_INDEX_SINGLE_THREADED_HPP_