#ifndef VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_
#define VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_

/*
 * VGGraph_Greedy: Greedy Variability Graph-based N-gram Selection
 * 
 * This implementation provides a greedy solution for n-gram selection using
 * recursive extension of frequent n-grams with set cover optimization.
 * The algorithm builds variable-length n-grams and selects an optimal subset
 * to cover query literals using a greedy set cover approach.
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <string>
#include <algorithm>
#include <future>
#include <limits>

#include "../../ngram_inverted_index.hpp"

namespace vggraph_greedy_index {

class VGGraph_Greedy : public NGramInvertedIndex {
 public:
    VGGraph_Greedy() = delete;
    VGGraph_Greedy(const VGGraph_Greedy &&) = delete;
    
    VGGraph_Greedy(const std::vector<std::string> & dataset, 
                   const std::vector<std::string> & queries,
                   float selectivity_threshold, 
                   int upper_n, 
                   int thread_count)
      : NGramInvertedIndex(dataset, queries),
        k_threshold_(selectivity_threshold),
        upper_n_(upper_n),
        thread_count_(thread_count),
        q_min_(2),
        max_gram_len_(upper_n) {}
    
    ~VGGraph_Greedy() {}

    void build_index(int upper_n = -1) override;
        
 protected:
    void select_grams(int upper_n = -1) override;

 private:
    const float k_threshold_;
    const int upper_n_;
    const int thread_count_;
    
    // Greedy algorithm parameters
    size_t q_min_;
    size_t max_gram_len_;
    
    // Type aliases for compatibility with reference implementation
    using RecordId = size_t;
    using PostingList = std::vector<RecordId>;

    // Core greedy algorithm methods (iterative approach with pruning)
    void extend_selected_grams(
        const std::unordered_map<std::string, PostingList>& current_grams,
        const std::set<std::string>& selected_grams,
        std::unordered_map<std::string, PostingList>& next_grams,
        size_t tau);
    
    void merge_index_maps(
        std::set<std::string>& out_keys,
        std::unordered_map<std::string, PostingList>& out_index,
        const std::set<std::string>& in_keys,
        const std::unordered_map<std::string, PostingList>& in_index);
    
    std::vector<std::string> vggraph_greedy_cover(
        const std::vector<std::string>& literals,
        const std::set<std::string>& index_keys,
        const std::unordered_map<std::string, PostingList>& index_map);
    
    size_t dynamic_tau(
        const std::unordered_map<std::string, PostingList>& grams, 
        double quantile);

    // Parallel processing methods
    void build_initial_ngrams_parallel(
        std::unordered_map<std::string, PostingList>& initial_grams);
        
    void process_chunk_for_initial_grams(
        size_t start, size_t end,
        std::unordered_map<std::string, PostingList>& thread_grams);
        
    void extend_grams_parallel(
        const std::unordered_map<std::string, PostingList>& current_grams,
        const std::set<std::string>& grams_to_extend,
        std::unordered_map<std::string, PostingList>& extended_grams,
        size_t tau);
};

} // namespace vggraph_greedy_index

#endif // VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_
