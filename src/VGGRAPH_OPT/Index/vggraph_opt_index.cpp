#include "vggraph_opt_index.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <future>
#include <iterator>

namespace vggraph_opt_index {

void VGGraph_Opt::build_index(int upper_n) {
    if (upper_n == -1) {
        upper_n = upper_n_;
    }
    
    std::cout << "Building VGGraph_Opt index with selectivity threshold: " 
              << selectivity_threshold_ << ", upper_n: " << upper_n 
              << ", thread_count: " << thread_count_ 
              << ", key_upper_bound: " << key_upper_bound_ << std::endl;
    
    // Step 1: Build the variability graph
    build_variability_graph(upper_n);
    
    // Step 2: Calculate selectivity for each n-gram
    calculate_node_selectivity();
    
    // Step 3: Build graph edges based on n-gram relationships
    build_graph_edges();
    
    // Step 4: Calculate centrality scores
    calculate_centrality_scores();
    
    // Step 5: Select optimal n-grams
    select_optimal_ngrams();
    
    // Step 6: Build the actual inverted index
    for (const auto& pair : graph_nodes_) {
        if (pair.second.selected) {
            k_index_keys_.insert(pair.first);
            k_index_[pair.first] = std::vector<size_t>(pair.second.document_ids.begin(), 
                                                     pair.second.document_ids.end());
            std::sort(k_index_[pair.first].begin(), k_index_[pair.first].end());
        }
    }
    
    std::cout << "VGGraph_Opt index built with " << k_index_keys_.size() << " n-grams" 
              << " (bound: " << key_upper_bound_ << ")" << std::endl;
}

bool VGGraph_Opt::get_all_idxs(const std::string & reg, std::vector<size_t> & container) const {
    container.clear();
    
    // Find all keys in the regex
    std::vector<std::string> found_keys = find_all_keys(reg);
    
    if (found_keys.empty()) {
        return false;
    }
    
    // Get the intersection of all posting lists
    std::set<size_t> result_set;
    bool first = true;
    
    for (const std::string& key : found_keys) {
        auto it = k_index_.find(key);
        if (it != k_index_.end()) {
            const std::vector<size_t>& posting_list = it->second;
            
            if (first) {
                result_set.insert(posting_list.begin(), posting_list.end());
                first = false;
            } else {
                std::set<size_t> current_set(posting_list.begin(), posting_list.end());
                std::set<size_t> intersection;
                std::set_intersection(result_set.begin(), result_set.end(),
                                    current_set.begin(), current_set.end(),
                                    std::inserter(intersection, intersection.begin()));
                result_set = intersection;
            }
        }
    }
    
    container.assign(result_set.begin(), result_set.end());
    return !container.empty();
}

void VGGraph_Opt::select_grams(int upper_n) {
    // This method is called by build_index, so we don't need to implement it separately
    // The gram selection logic is integrated into build_index
}

void VGGraph_Opt::build_variability_graph(int max_n) {
    const int num_threads = std::min(thread_count_, static_cast<int>(k_dataset_size_));
    const int docs_per_thread = k_dataset_size_ / num_threads;
    
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < num_threads; ++i) {
        int start_idx = i * docs_per_thread;
        int end_idx = (i == num_threads - 1) ? k_dataset_size_ : (i + 1) * docs_per_thread;
        
        futures.push_back(std::async(std::launch::async, 
                                   &VGGraph_Opt::process_documents_parallel, 
                                   this, start_idx, end_idx, max_n));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.get();
    }
}

void VGGraph_Opt::process_documents_parallel(int start_idx, int end_idx, int max_n) {
    std::unordered_map<std::string, std::set<size_t>> local_ngrams;
    
    for (int doc_id = start_idx; doc_id < end_idx; ++doc_id) {
        extract_ngrams_from_document(k_dataset_[doc_id], max_n, local_ngrams, doc_id);
    }
    
    // Merge results into global structure
    merge_local_results(local_ngrams);
}

void VGGraph_Opt::extract_ngrams_from_document(const std::string& document, 
                                              int max_n, 
                                              std::unordered_map<std::string, std::set<size_t>>& local_ngrams,
                                              size_t doc_id) {
    for (int n = 1; n <= max_n; ++n) {
        std::vector<std::string> ngrams = generate_ngrams(document, n);
        for (const std::string& ngram : ngrams) {
            local_ngrams[ngram].insert(doc_id);
        }
    }
}

std::vector<std::string> VGGraph_Opt::generate_ngrams(const std::string& text, int n) {
    std::vector<std::string> ngrams;
    if (text.length() < n) {
        return ngrams;
    }
    
    for (size_t i = 0; i <= text.length() - n; ++i) {
        ngrams.push_back(text.substr(i, n));
    }
    
    return ngrams;
}

void VGGraph_Opt::merge_local_results(const std::unordered_map<std::string, std::set<size_t>>& local_ngrams) {
    std::lock_guard<std::mutex> lock(graph_mutex_);
    
    for (const auto& pair : local_ngrams) {
        const std::string& ngram = pair.first;
        const std::set<size_t>& doc_ids = pair.second;
        
        if (graph_nodes_.find(ngram) == graph_nodes_.end()) {
            graph_nodes_[ngram] = GraphNode(ngram);
        }
        
        graph_nodes_[ngram].document_ids.insert(doc_ids.begin(), doc_ids.end());
    }
}

void VGGraph_Opt::calculate_node_selectivity() {
    for (auto& pair : graph_nodes_) {
        GraphNode& node = pair.second;
        node.selectivity = static_cast<double>(node.document_ids.size()) / k_dataset_size_;
    }
}

void VGGraph_Opt::build_graph_edges() {
    std::vector<std::string> ngrams;
    for (const auto& pair : graph_nodes_) {
        ngrams.push_back(pair.first);
    }
    
    // Build edges based on Jaccard similarity
    for (size_t i = 0; i < ngrams.size(); ++i) {
        for (size_t j = i + 1; j < ngrams.size(); ++j) {
            const std::string& ngram1 = ngrams[i];
            const std::string& ngram2 = ngrams[j];
            
            double similarity = calculate_jaccard_similarity(
                graph_nodes_[ngram1].document_ids,
                graph_nodes_[ngram2].document_ids
            );
            
            // Add edge if similarity is above threshold
            if (similarity > 0.1) {  // Threshold for graph connectivity
                graph_nodes_[ngram1].neighbors.push_back(ngram2);
                graph_nodes_[ngram2].neighbors.push_back(ngram1);
            }
        }
    }
}

double VGGraph_Opt::calculate_jaccard_similarity(const std::set<size_t>& set1, 
                                                const std::set<size_t>& set2) {
    std::set<size_t> intersection;
    std::set_intersection(set1.begin(), set1.end(),
                         set2.begin(), set2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<size_t> union_set;
    std::set_union(set1.begin(), set1.end(),
                   set2.begin(), set2.end(),
                   std::inserter(union_set, union_set.begin()));
    
    if (union_set.empty()) return 0.0;
    return static_cast<double>(intersection.size()) / union_set.size();
}

void VGGraph_Opt::calculate_centrality_scores() {
    // Calculate betweenness centrality for each node
    for (auto& pair : graph_nodes_) {
        const std::string& ngram = pair.first;
        pair.second.centrality_score = calculate_betweenness_centrality(ngram);
    }
}

double VGGraph_Opt::calculate_betweenness_centrality(const std::string& ngram) {
    // Simplified betweenness centrality calculation
    // In a full implementation, this would use algorithms like Brandes' algorithm
    const GraphNode& node = graph_nodes_[ngram];
    
    // For simplicity, use degree centrality weighted by selectivity
    double degree = static_cast<double>(node.neighbors.size());
    double selectivity_weight = 1.0 - node.selectivity;  // Lower selectivity = higher weight
    
    return degree * selectivity_weight;
}

void VGGraph_Opt::select_optimal_ngrams() {
    // Use greedy selection algorithm with coverage optimization
    greedy_selection_with_coverage();
}

void VGGraph_Opt::greedy_selection_with_coverage() {
    // Check for valid key upper bound
    if (key_upper_bound_ <= 0) {
        std::cout << "Warning: key_upper_bound_ is " << key_upper_bound_ 
                  << ", no n-grams will be selected" << std::endl;
        return;
    }
    
    std::vector<std::pair<std::string, double>> candidates;
    
    // Filter candidates by selectivity threshold
    for (const auto& pair : graph_nodes_) {
        if (pair.second.selectivity <= selectivity_threshold_) {
            double score = pair.second.centrality_score / (pair.second.selectivity + 1e-10);
            candidates.push_back({pair.first, score});
        }
    }
    
    // Sort by score (descending)
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::cout << "Found " << candidates.size() << " candidate n-grams after selectivity filtering" << std::endl;
    std::cout << "Key upper bound: " << key_upper_bound_ << std::endl;
    
    // Adjust expectations if we have fewer candidates than the bound
    size_t effective_bound = std::min(static_cast<size_t>(key_upper_bound_), candidates.size());
    std::cout << "Effective selection bound: " << effective_bound << std::endl;

    std::set<std::string> selected_ngrams;
    std::set<size_t> covered_documents;
    
    // Greedy selection with key upper bound constraint
    for (const auto& candidate : candidates) {
        const std::string& ngram = candidate.first;
        const GraphNode& node = graph_nodes_[ngram];
        
        // Check if we've reached the key upper bound
        if (selected_ngrams.size() >= effective_bound) {
            std::cout << "Reached effective bound of " << effective_bound << " n-grams" << std::endl;
            break;
        }
        
        // Calculate additional coverage
        std::set<size_t> new_coverage;
        std::set_difference(node.document_ids.begin(), node.document_ids.end(),
                           covered_documents.begin(), covered_documents.end(),
                           std::inserter(new_coverage, new_coverage.begin()));
        
        // Select if it provides significant additional coverage
        if (!new_coverage.empty() && 
            (selected_ngrams.size() < effective_bound && 
             (selected_ngrams.size() < 100 || new_coverage.size() > 1))) {
            selected_ngrams.insert(ngram);
            covered_documents.insert(node.document_ids.begin(), node.document_ids.end());
            graph_nodes_[ngram].selected = true;
        }
        
        // Stop if we have good coverage or reached the limit
        if (covered_documents.size() >= k_dataset_size_ * 0.9 || 
            selected_ngrams.size() >= effective_bound) {
            break;
        }
    }
    
    std::cout << "Selected " << selected_ngrams.size() << " n-grams covering " 
              << covered_documents.size() << " documents" 
              << " (limit: " << key_upper_bound_ << ")" << std::endl;
}

bool VGGraph_Opt::has_sufficient_coverage(const std::set<std::string>& selected_ngrams) {
    std::set<size_t> covered_documents;
    
    for (const std::string& ngram : selected_ngrams) {
        const auto& doc_ids = graph_nodes_[ngram].document_ids;
        covered_documents.insert(doc_ids.begin(), doc_ids.end());
    }
    
    double coverage_ratio = static_cast<double>(covered_documents.size()) / k_dataset_size_;
    return coverage_ratio >= 0.8;  // 80% coverage threshold
}

double VGGraph_Opt::calculate_coverage_score(const std::set<std::string>& selected_ngrams) {
    std::set<size_t> covered_documents;
    
    for (const std::string& ngram : selected_ngrams) {
        const auto& doc_ids = graph_nodes_[ngram].document_ids;
        covered_documents.insert(doc_ids.begin(), doc_ids.end());
    }
    
    return static_cast<double>(covered_documents.size()) / k_dataset_size_;
}

} // namespace vggraph_opt_index
