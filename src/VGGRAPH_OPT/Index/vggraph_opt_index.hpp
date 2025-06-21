#ifndef VGGRAPH_OPT_INDEX_VGGRAPH_OPT_INDEX_HPP_
#define VGGRAPH_OPT_INDEX_VGGRAPH_OPT_INDEX_HPP_

/*
 * VGGraph_Opt: Optimal Variability Graph-based N-gram Selection
 * 
 * This implementation provides an optimal (NP-hard) solution for n-gram 
 * selection using graph-based centrality measures and coverage optimization.
 * The algorithm finds theoretically optimal n-gram subsets for regex indexing
 * at the cost of increased computational complexity.
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

namespace vggraph_opt_index {

class VGGraph_Opt : public NGramInvertedIndex {
 public:
    VGGraph_Opt() = delete;
    VGGraph_Opt(const VGGraph_Opt &&) = delete;
    
    VGGraph_Opt(const std::vector<std::string> & dataset, 
                 float selectivity_threshold, 
                 int upper_n, 
                 int thread_count)
      : NGramInvertedIndex(dataset),
        selectivity_threshold_(selectivity_threshold),
        upper_n_(upper_n),
        thread_count_(thread_count) {}
    
    VGGraph_Opt(const std::vector<std::string> & dataset, 
                 const std::vector<std::string> & queries,
                 float selectivity_threshold, 
                 int upper_n, 
                 int thread_count)
      : NGramInvertedIndex(dataset, queries),
        selectivity_threshold_(selectivity_threshold),
        upper_n_(upper_n),
        thread_count_(thread_count) {}
    
    ~VGGraph_Opt() {}

    void build_index(int upper_n = -1) override;
    
 protected:
    void select_grams(int upper_n = -1) override;

 private:
    const float selectivity_threshold_;
    const int upper_n_;
    const int thread_count_;

    // Graph-based structures for n-gram selection
    struct GraphNode {
        std::string ngram;
        double selectivity;
        std::set<size_t> document_ids;
        std::vector<std::string> neighbors;
        double centrality_score;
        bool selected;
        
        GraphNode() : selectivity(0.0), centrality_score(0.0), selected(false) {}
        GraphNode(const std::string& gram) : ngram(gram), selectivity(0.0), 
                                           centrality_score(0.0), selected(false) {}
    };

    std::unordered_map<std::string, GraphNode> graph_nodes_;
    std::mutex graph_mutex_;

    // Helper methods for VGGraph algorithm
    void build_variability_graph(int max_n);
    void calculate_node_selectivity();
    void calculate_centrality_scores();
    void select_optimal_ngrams();
    
    // Multi-threaded processing helpers
    void process_documents_parallel(int start_idx, int end_idx, int max_n);
    void extract_ngrams_from_document(const std::string& document, 
                                    int max_n, 
                                    std::unordered_map<std::string, std::set<size_t>>& local_ngrams,
                                    size_t doc_id);
    
    // Graph analysis methods
    void build_graph_edges();
    double calculate_jaccard_similarity(const std::set<size_t>& set1, 
                                      const std::set<size_t>& set2);
    double calculate_betweenness_centrality(const std::string& ngram);
    
    // Selection optimization
    void greedy_selection_with_coverage();
    bool has_sufficient_coverage(const std::set<std::string>& selected_ngrams);
    double calculate_coverage_score(const std::set<std::string>& selected_ngrams);
    
    // Utility methods
    std::vector<std::string> generate_ngrams(const std::string& text, int n);
    void merge_local_results(const std::unordered_map<std::string, std::set<size_t>>& local_ngrams);
};

} // namespace vggraph_opt_index

#endif // VGGRAPH_OPT_INDEX_VGGRAPH_OPT_INDEX_HPP_
