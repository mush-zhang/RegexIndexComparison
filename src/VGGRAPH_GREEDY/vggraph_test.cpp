#include "vggraph_greedy_index.hpp"
#include <iostream>
#include <vector>
#include <string>

int main() {
    // Small test dataset
    std::vector<std::string> dataset = {
        "banana",
        "bandana",
        "cabana",
        "anagram"
    };

    // Example regex queries (as strings)
    std::vector<std::string> queries = {
        "ba(.*)a",
        "(.*)ana",
        "c(.*)a",
        "ana(.*)"
    };

    float selectivity_threshold = 0.5f;
    int upper_n = 3;         // Maximum n-gram length
    int thread_count = 2;    // Number of threads

    vggraph_greedy_index::VGGraph_Greedy index(dataset, queries, selectivity_threshold, upper_n, thread_count);

    index.build_index(upper_n);

    std::cout << "VGGraph Greedy Index:" << std::endl;
    index.print_index(false);

    return 0;
}