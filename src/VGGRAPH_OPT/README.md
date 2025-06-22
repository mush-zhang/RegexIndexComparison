# VGGraph_Opt Index

VGGraph_Opt (Optimal Variability Graph-based) is a novel n-gram selection method for regex indexing that builds a graph-based representation of n-gram relationships to optimize index performance. This is the optimal version that implements an NP-hard algorithm for n-gram selection.

## Overview

The VGGraph algorithm creates a variability graph where:
- **Nodes** represent n-grams extracted from the dataset
- **Edges** connect n-grams based on their document overlap (Jaccard similarity)
- **Selection** is performed using centrality measures and coverage optimization

## Algorithm Steps

1. **Graph Construction**: Build a variability graph with n-grams as nodes (NP-hard optimization)
2. **Selectivity Calculation**: Compute selectivity for each n-gram (document frequency / total documents)
3. **Edge Building**: Connect n-grams with high Jaccard similarity
4. **Centrality Analysis**: Calculate betweenness centrality scores using optimal algorithms
5. **Optimal Selection**: Use advanced optimization techniques for n-gram selection

## Parameters

- **selectivity_threshold**: Maximum allowed selectivity for selected n-grams (float)
- **upper_n**: Maximum n-gram length to consider (int)
- **thread_count**: Number of threads for parallel processing (int)

## Key Features

- **Multi-threaded processing** for scalable index construction
- **Graph-based analysis** to identify important n-grams
- **Coverage optimization** to ensure good query performance
- **Selectivity filtering** to control index size

## Usage

```cpp
#include "Index/vggraph_index.hpp"

// Create index with parameters
VGGraph_Opt index(dataset, queries, 0.1f, 4, 8);

// Build the index
index.build_index();

// Use for querying
std::vector<size_t> results;
index.get_all_idxs(query, results);
```

## Implementation Details

### Graph Node Structure
```cpp
struct GraphNode {
    std::string ngram;           // The n-gram string
    double selectivity;          // Document frequency ratio
    std::set<size_t> document_ids;  // Documents containing this n-gram
    std::vector<std::string> neighbors;  // Connected n-grams
    double centrality_score;     // Importance score
    bool selected;              // Whether included in final index
};
```

### Selection Strategy

The algorithm uses a greedy approach:
1. Filter candidates by selectivity threshold
2. Score candidates using centrality/selectivity ratio
3. Iteratively select n-grams that provide maximum additional coverage
4. Stop when sufficient coverage is achieved (90% by default)

## Performance Characteristics

- **Build Time**: O(D·N·T + G²) where D=documents, N=max n-gram length, T=threads, G=unique n-grams
- **Memory Usage**: O(G·D) for storing document sets per n-gram
- **Query Time**: Similar to standard inverted index lookup

## Comparison with Other Methods

| Method | Selection Strategy | Strengths | Weaknesses |
|--------|-------------------|-----------|------------|
| FREE | Selectivity-based | Simple, fast | May miss important relationships |
| BEST | Workload-aware | Query-optimized | Requires query workload |
| LPMS | Linear programming | Optimal solution | Computationally expensive |
| VGGraph_Opt | Optimal graph centrality | Theoretically optimal coverage | NP-hard complexity |

## Tuning Guidelines

- **selectivity_threshold**: Lower values (0.01-0.1) for large datasets, higher (0.1-0.3) for smaller ones
- **upper_n**: Typically 3-5, higher values increase computational cost
- **thread_count**: Set to number of CPU cores for optimal performance
