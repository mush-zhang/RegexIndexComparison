#ifndef BEST_INDEX_SINGLE_THREADED_HPP_
#define BEST_INDEX_SINGLE_THREADED_HPP_

#include <map>

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
    enum dist_type { kMaxDevDist1, kMaxDevDist2, kMaxDevDist3, kInvalid };

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
        std::vector<std::set<unsigned int>> qg_list;
        std::vector<std::vector<unsigned int>> gr_list;
        std::vector<unsigned int> rc;
    };

    SingleThreadedIndex() = delete;
    SingleThreadedIndex(const SingleThreadedIndex &&) = delete;
    SingleThreadedIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold)
      : NGramInvertedIndex(dataset), 
        k_queries_(queries), k_queries_size_(queries.size()),
        k_threshold_(sel_threshold), 
        k_reduced_queries_size_(queries.size()),
        k_original_queries_(queries) {}
    
    SingleThreadedIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold, long workload_reduced_size,
               dist_type dist_measure_type)
      : NGramInvertedIndex(dataset), 
        k_queries_(queries), k_queries_size_(queries.size()),
        k_threshold_(sel_threshold), 
        k_reduced_queries_size_(workload_reduced_size),
        dist_measure_type_(dist_measure_type),
        k_original_queries_(queries) {}
    
    ~SingleThreadedIndex() {}

    // No constraint on size of gram in BEST, use build_index();
    void build_index(int upper_k=-1) override;

    void set_max_num_keys(int max_num_keys) { k_max_num_keys_ = max_num_keys; }

 protected:
    dist_type dist_measure_type_ = dist_type::kInvalid;
    int k_max_num_keys_ = -1;

    void select_grams(int upper_k=-1) override;

    std::vector<std::string> candidate_gram_set_gen(
        std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, unsigned int> & pre_suf_count);

    std::vector<std::vector<std::string>> get_query_literals();

    std::map<std::string, unsigned int> get_all_gram_counts(
        const std::vector<std::vector<std::string>> & query_literals);
    
    std::unordered_map<unsigned int, std::vector<size_t>> k_medians(
        const std::vector<std::vector<double>> & dist_mtx, 
        int num_queries, int num_clusters);
    
    std::vector<std::set<std::string>> get_all_multigrams_per_query(
        const std::vector<std::vector<std::string>> & query_literals);
    
    std::vector<std::vector<double>> calculate_pairwise_dist(
        const std::vector<std::set<std::string>> & qg_gram_set,
        const std::map<std::string, unsigned int> & pre_suf_count);
    
    void compute_benefit(std::vector<long double> & benefit, 
        const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job, 
        size_t num_queries);
    
    bool all_covered(const std::set<unsigned int> & index,
        const best_index::SingleThreadedIndex::job & job,  
        size_t query_size);

    void build_job(best_index::SingleThreadedIndex::job & job,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals);

    void build_job_local(best_index::SingleThreadedIndex::job & job,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals,
        const std::vector<size_t> r_list);

 private:
    const long double k_queries_size_;
    const std::vector<std::string> & k_queries_;

    long double k_reduced_queries_size_;

    const std::vector<std::string> & k_original_queries_;

    /** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;

    /** Helpers **/
    void workload_reduction(std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, unsigned int> & pre_suf_count);
    
    bool index_covered(const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job,
        size_t r_j, size_t q_k);
};

} // namespace best_index

#endif // BEST_INDEX_SINGLE_THREADED_HPP_