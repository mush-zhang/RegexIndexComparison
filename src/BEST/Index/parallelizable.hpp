#ifndef BEST_INDEX_PARALLELIZABLE_HPP_
#define BEST_INDEX_PARALLELIZABLE_HPP_

#include "single_threaded.hpp"

namespace best_index {
class ParallelizableIndex : public SingleThreadedIndex {
 public:
    ParallelizableIndex() = delete;
    ParallelizableIndex(const ParallelizableIndex &&) = delete;
    ParallelizableIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold)
      : SingleThreadedIndex(dataset, queries, sel_threshold),
        k_num_clusters_(queries.size()/5) {
            dist_measure_type_ = dist_type::kMaxDevDist2;
        }
    
    ParallelizableIndex(const std::vector<std::string> & dataset, 
               const std::vector<std::string> & queries, 
               double sel_threshold, long workload_reduced_size,
               dist_type dist_measure_type)
      : SingleThreadedIndex(dataset, queries, sel_threshold,
                            workload_reduced_size, dist_measure_type),
        k_num_clusters_(queries.size()/5) {}

    ~ParallelizableIndex() {}
 private:
    void select_grams(int upper_k=-1) override;
    
    /**Accoriding to Section 3 part 3), we choose a hard-coded value
       k between |Q|/2 and |Q|/5 **/
    const unsigned int k_num_clusters_;

    std::vector<std::set<std::string>>get_all_multigrams_per_query(
        const std::vector<std::vector<std::string>> & query_literals) ;
};

} // namespace best_index

#endif // BEST_INDEX_PARALLELIZABLE_HPP_