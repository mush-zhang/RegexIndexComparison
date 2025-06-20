#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <climits>

#include "Index/vggraph_opt_index.hpp"
#include "../simple_query_matcher.hpp"

using namespace vggraph_opt_index;

int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " <dataset_file> <queries_file> <selectivity_threshold> <upper_n> <thread_count> [key_upper_bound]" << std::endl;
        return 1;
    }

    std::string dataset_file = argv[1];
    std::string queries_file = argv[2];
    float selectivity_threshold = std::stof(argv[3]);
    int upper_n = std::stoi(argv[4]);
    int thread_count = std::stoi(argv[5]);
    
    // Optional key upper bound parameter
    long long int key_upper_bound = LLONG_MAX;
    if (argc >= 7) {
        key_upper_bound = std::stoll(argv[6]);
    }

    // Load dataset
    std::vector<std::string> dataset;
    std::ifstream dataset_stream(dataset_file);
    if (!dataset_stream.is_open()) {
        std::cerr << "Error: Could not open dataset file: " << dataset_file << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(dataset_stream, line)) {
        dataset.push_back(line);
    }
    dataset_stream.close();

    // Load queries
    std::vector<std::string> queries;
    std::ifstream queries_stream(queries_file);
    if (!queries_stream.is_open()) {
        std::cerr << "Error: Could not open queries file: " << queries_file << std::endl;
        return 1;
    }

    while (std::getline(queries_stream, line)) {
        queries.push_back(line);
    }
    queries_stream.close();

    std::cout << "Loaded " << dataset.size() << " documents and " << queries.size() << " queries" << std::endl;
    std::cout << "Key upper bound: " << key_upper_bound << std::endl;

    // Create VGGraph_Opt index
    VGGraph_Opt index(dataset, queries, selectivity_threshold, upper_n, thread_count);
    index.set_key_upper_bound(key_upper_bound);

    // Build index
    auto start_time = std::chrono::high_resolution_clock::now();
    index.build_index(upper_n);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Index build time: " << build_duration.count() << " ms" << std::endl;
    std::cout << "Index size: " << index.get_num_keys() << " n-grams" << std::endl;
    std::cout << "Memory usage: " << index.get_bytes_used() << " bytes" << std::endl;

    // Test queries
    SimpleQueryMatcher matcher(index);
    int total_matches = 0;
    
    start_time = std::chrono::high_resolution_clock::now();
    for (const auto& query : queries) {
        std::vector<size_t> results;
        if (index.get_all_idxs(query, results)) {
            total_matches += results.size();
        }
    }
    end_time = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "Query processing time: " << query_duration.count() << " microseconds" << std::endl;
    std::cout << "Total matches found: " << total_matches << std::endl;
    std::cout << "Average matches per query: " << static_cast<double>(total_matches) / queries.size() << std::endl;

    // Print index statistics
    index.print_index(true);

    return 0;
}
