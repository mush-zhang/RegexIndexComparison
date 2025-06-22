#ifndef VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_
#define VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_

/*
 * VGGraph_Greedy: Greedy Variability Graph-based N-gram Selection
 * 
 * This implementation provides a greedy solution for n-gram selection using
 * iterative extension of frequent n-grams. The algorithm starts with minimum
 * length n-grams and extends those that are too frequent, keeping those that
 * meet the selectivity threshold.
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <string>
#include <algorithm>

#include "../../ngram_inverted_index.hpp"

namespace vggraph_greedy_index {

class VGGraph_Greedy : public NGramInvertedIndex {
 public:
    VGGraph_Greedy() = delete;
    VGGraph_Greedy(const VGGraph_Greedy &&) = delete;
    
    VGGraph_Greedy(const std::vector<std::string> & dataset, 
                   float selectivity_threshold, 
                   int upper_n, 
                   int thread_count)
      : NGramInvertedIndex(dataset),
        k_threshold_(selectivity_threshold),
        upper_n_(upper_n),
        thread_count_(thread_count),
        q_min_(2),
        max_gram_len_(upper_n),
        append_terminal_(false) {}
    
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
        max_gram_len_(upper_n),
        append_terminal_(false) {}
    
    ~VGGraph_Greedy() {}

    void build_index(int upper_n = -1) override;
        
 protected:
    void select_grams(int upper_n = -1) override;

 private:
    const float k_threshold_;
    const int upper_n_;
    const int thread_count_;
    
    // Greedy algorithm parameters (following VGramIndex pattern)
    int q_min_;
    int max_gram_len_;
    bool append_terminal_;
    std::string alphabet_;
    
    // Extension tracking for greedy growth (optional, for debugging)
    std::unordered_map<std::string, std::set<std::string>> gram_ext_map_;

    // Core greedy algorithm methods (following reference implementation)
    void find_alphabet();
    
    // Parallel processing methods
    void build_initial_ngrams_parallel(
        std::unordered_map<std::string, std::vector<size_t>>& next_index,
        std::set<std::string>& current_grams);
    
    void process_grams_parallel(
        const std::set<std::string>& current_grams,
        const std::unordered_map<std::string, std::vector<size_t>>& next_index,
        std::unordered_map<std::string, std::vector<size_t>>& new_index,
        std::set<std::string>& next_grams,
        size_t tau,
        int current_len);
};

} // namespace vggraph_greedy_index

#endif // VGGRAPH_GREEDY_INDEX_VGGRAPH_GREEDY_INDEX_HPP_
