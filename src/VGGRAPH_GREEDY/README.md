# VGGraph_Greedy: Greedy Variability Graph-based N-gram Selection

## Overview

VGGraph_Greedy implements a greedy algorithm for n-gram selection in regex indexing using variability graph-based centrality measures. This approach provides an efficient alternative to the optimal VGGraph_Opt method, trading theoretical optimality for improved computational performance.

## Algorithm Description

The greedy VGGraph algorithm works as follows:

1. **Alphabet Discovery**: Extracts the character alphabet from the input dataset
2. **N-gram Generation**: Generates all possible n-grams from length `q_min` to `upper_n`
3. **Variability Graph Construction**: Builds a graph where nodes are n-grams and edges represent document co-occurrence relationships
4. **Selectivity Calculation**: Computes selectivity scores for each n-gram based on document frequency
5. **Centrality Analysis**: Calculates centrality scores using simplified betweenness centrality
6. **Greedy Selection**: Iteratively selects n-grams with highest marginal utility until the desired number is reached

## Key Features

- **Greedy Optimization**: Efficient selection algorithm with polynomial time complexity
- **Selectivity Filtering**: Filters n-grams based on configurable selectivity threshold
- **Multithreaded Processing**: Parallel document processing for improved performance
- **Coverage-based Utility**: Uses marginal coverage contribution for selection decisions
- **Key Upper Bound**: Supports limiting the total number of selected n-grams

## Parameters

- `selectivity_threshold`: Maximum selectivity (document frequency ratio) for n-gram inclusion
- `upper_n`: Maximum n-gram length to consider
- `thread_count`: Number of threads for parallel processing
- `key_upper_bound`: Maximum number of n-grams to select (optional)

## Usage

### Building the Test Executable

```bash
make test_vggraph_greedy.out
```

### Running Tests

```bash
./test_vggraph_greedy.out <dataset_file> <queries_file> <selectivity_threshold> <upper_n> <thread_count> [key_upper_bound]
```

### Example

```bash
./test_vggraph_greedy.out test_dataset.txt test_queries.txt 0.1 4 8 1000
```

This command:
- Uses `test_dataset.txt` as the document dataset
- Uses `test_queries.txt` as the query set
- Sets selectivity threshold to 0.1 (10% of documents)
- Considers n-grams up to length 4
- Uses 8 threads for parallel processing
- Limits selection to 1000 n-grams maximum

## Implementation Details

### Graph Construction

The variability graph is constructed by:
1. Creating nodes for each unique n-gram
2. Adding edges between n-grams with high Jaccard similarity in document sets
3. Computing centrality scores based on graph topology

### Greedy Selection Strategy

The greedy algorithm selects n-grams by:
1. Filtering candidates based on selectivity threshold
2. Computing marginal utility for each candidate
3. Selecting the candidate with highest utility
4. Updating the candidate set and repeating

### Utility Function

The utility function combines:
- **Coverage Contribution**: Number of new documents covered
- **Centrality Score**: Graph-based importance measure
- **Selectivity Factor**: Inverse of document frequency

## Performance Characteristics

- **Time Complexity**: O(n²) for graph construction, O(nk) for greedy selection
- **Space Complexity**: O(n) for n-gram storage plus graph structure
- **Scalability**: Suitable for large datasets with moderate n-gram vocabularies

## Comparison with VGGraph_Opt

| Aspect | VGGraph_Greedy | VGGraph_Opt |
|--------|----------------|-------------|
| Algorithm | Greedy | Optimal (NP-hard) |
| Time Complexity | Polynomial | Exponential |
| Solution Quality | Locally optimal | Globally optimal |
| Scalability | High | Limited |
| Use Case | Large datasets | Small to medium datasets |

## Testing and Validation

The implementation includes comprehensive testing infrastructure:

- **Unit Tests**: Individual component testing
- **Integration Tests**: Full pipeline validation
- **Performance Tests**: Scalability and efficiency measurement
- **Correctness Tests**: Result quality verification

## Files Structure

```
VGGRAPH_GREEDY/
├── Index/
│   ├── vggraph_greedy_index.hpp    # Header file
│   └── vggraph_greedy_index.cpp    # Implementation
├── test_main.cpp                   # Test driver
├── Makefile                        # Build configuration
└── README.md                       # This file
```

## Dependencies

- C++17 or later
- pthread (for multithreading)
- RE2 library (for regex processing)
- Standard Template Library (STL)

## Integration

The VGGraph_Greedy class inherits from `NGramInvertedIndex` and can be used as a drop-in replacement for other n-gram selection methods in the regex indexing framework.

## Future Enhancements

- Advanced centrality measures (PageRank, eigenvector centrality)
- Adaptive threshold selection
- Online learning capabilities
- Distributed processing support
