#ifndef BEST_INDEX_SINGLE_THREADED_HPP_
#define BEST_INDEX_SINGLE_THREADED_HPP_

#include <map>
#include <unordered_map>

#include "../../ngram_btree_index.hpp"

namespace best_index {

enum dist_type { kMaxDevDist1, kMaxDevDist2, kMaxDevDist3, kInvalid };

/**
 * Note: the orginal paper assumes b+tree for size constraint calculation.
 *       Yet it does not actually build the index and measure query performance.
 *       It uses the ARE (false positives related measurement) to measure the.
 *       effectiveness of index
 */
class SingleThreadedIndex  : public NGramBtreeIndex {
 public:

    /**
     * Q-G-list: vector in order of q, where each element is
     *      set of idx of g \in candidates s.t. g \in q
     * G-R-list: vector in order of g, where each element is
     *      set of idx of r \in candidates s.t. g \in r
     *   Note: to avoid multiple scans of the dataset, 
     *        build R-G-list first
     * R_c: set of idx of r s.t. \exists some g \in r
     */
    struct job {
        std::vector<std::set<size_t>> qg_list;
        std::vector<std::vector<size_t>> gr_list;
        std::vector<size_t> rc;
    };

    SingleThreadedIndex() = delete;
    SingleThreadedIndex(const SingleThreadedIndex &&) = delete;
    SingleThreadedIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold)
      : NGramBtreeIndex(dataset, queries),
        k_threshold_(sel_threshold), 
        k_reduced_queries_size_(queries.size()) {}
    
    SingleThreadedIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold, long workload_reduced_size,
               dist_type dist_measure_type)
      : NGramBtreeIndex(dataset, queries),
        k_threshold_(sel_threshold), 
        k_reduced_queries_size_(workload_reduced_size),
        dist_measure_type_(dist_measure_type) {}
    
    ~SingleThreadedIndex() {}

    // No constraint on size of gram in BEST, use build_index();
    void build_index(int upper_n=-1) override;

    void set_max_iteration(long max_iter) { max_iteration_ = max_iter; }

 protected:
    dist_type dist_measure_type_ = dist_type::kInvalid;

    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;

    void select_grams(int upper_n=-1) override;

    std::vector<std::string> candidate_gram_set_gen(
        std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, size_t> & pre_suf_count);

    std::map<std::string, size_t> get_all_gram_counts(
        const std::vector<std::vector<std::string>> & query_literals);
    
    std::unordered_map<size_t, std::vector<size_t>> k_medians(
        const std::vector<std::vector<double>> & dist_mtx, 
        int num_queries, int num_clusters);
    
    std::vector<std::set<std::string>> get_all_multigrams_per_query(
        const std::vector<std::vector<std::string>> & query_literals);
    
    std::vector<std::vector<double>> calculate_pairwise_dist(
        const std::vector<std::set<std::string>> & qg_gram_set,
        const std::map<std::string, size_t> & pre_suf_count);
    
    void compute_benefit(std::vector<long double> & benefit, 
        const std::set<size_t> & index, 
        const best_index::SingleThreadedIndex::job & job, 
        size_t num_queries);
    
    bool all_covered(const std::set<size_t> & index,
        const best_index::SingleThreadedIndex::job & job,  
        size_t query_size);
    
    void indexed_grams_in_string(const std::string & l, 
        const std::vector<std::string> & candidates,
        std::vector<std::set<size_t>> & g_list, 
        size_t idx,
        const std::vector<bool> & candidates_filter=std::vector<bool>());

    void indexed_grams_in_literals(const std::vector<std::string> & literals, 
        const std::vector<std::string> & candidates,
        std::vector<std::set<size_t>> & g_list,
        size_t idx);
    
    void build_gr_list_rc(best_index::SingleThreadedIndex::job & job, 
        size_t candidates_size,  
        size_t dataset_size,
        const std::vector<std::set<size_t>> & rg_list);

 private:
    long double k_reduced_queries_size_;
    long max_iteration_ = 100000;

    /** Helpers **/
    void workload_reduction(std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, size_t> & pre_suf_count);

    bool index_covered(const std::set<size_t> & index, 
        const best_index::SingleThreadedIndex::job & job,
        size_t r_j, size_t q_k);

    void build_qg_list(std::vector<std::set<size_t>> & qg_list,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals);
    
    void build_job(best_index::SingleThreadedIndex::job & job,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals);

};

} // namespace best_index

#endif // BEST_INDEX_SINGLE_THREADED_HPP_