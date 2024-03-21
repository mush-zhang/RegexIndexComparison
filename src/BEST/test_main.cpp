#include "Index/naive.hpp"

#include <iostream> 
#include <cassert>

const double k_number_repeat = 10;

template <typename T>
bool compare_lists(const std::vector<T> & a, const std::vector<T> & b) {
    if (a.size() != b.size()) return false;
    std::unordered_set<T> as(a.begin(), a.end());
    std::unordered_set<T> bs(b.begin(), b.end());
    if (as.size() != a.size() || bs.size() != b.size()) return false;
    return as == bs;
}

void make_dataset_with_keys(const std::vector<std::string> & keys, 
                            std::vector<std::string> & dataset_container, 
                            double & threshold_constainer) {
    for (const auto key : keys) {
        dataset_container.emplace_back(key);
        for (size_t i = 0; i < key.size(); i++) {
            for (size_t j = i; j < key.size(); j++) {
                if (i == 0 && j == key.size()-1) {
                    break;
                }
                for (size_t k = 0; k < k_number_repeat; k++) {
                    dataset_container.push_back(key.substr(i, j-i+1));
                }
            }
        }
    }
    threshold_constainer = (k_number_repeat-1)/dataset_container.size();
}

void simple_index() {
    std::vector<std::string> test_query({
        "an",
        "York",
        "Franc",
        "kane",
    });
    std::vector<std::string> test_dataset;

    auto pi = best_index::NaiveIndex(test_dataset, test_query, 1);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    // assert(compare_lists(pi.get_line_pos_at("0"), {0}) && 
    //        compare_lists(pi.get_line_pos_at("1"), {1}) && 
    //        compare_lists(pi.get_line_pos_at("2"), {2}) && 
    //        compare_lists(pi.get_line_pos_at("3"), {3}) && 
    //        compare_lists(pi.get_line_pos_at("4"), {4}) &&
    //        compare_lists(pi.get_line_pos_at("5"), {5}) &&  
    //         "Line index should be in keys");

    // assert(compare_lists(pi.get_line_pos_at("a"), {0}) && 
    //        compare_lists(pi.get_line_pos_at("b"), {1}) && 
    //        compare_lists(pi.get_line_pos_at("c"), {2}) && 
    //        compare_lists(pi.get_line_pos_at("d"), {3}) && 
    //        compare_lists(pi.get_line_pos_at("e"), {4}) &&
    //        compare_lists(pi.get_line_pos_at("f"), {5}) &&  
    //         "Alphabet char should have their respecting line idx and ony the one line idx");
    
    // assert(compare_lists(pi.get_line_pos_at("."), {0, 1, 2, 3, 4, 5}) && 
    //        "Char . in all 0-5 lines");
}

int main() {
    std::cout << "BEGIN INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "\t GRAM CANDIDATE SET -------------------------------------------" << std::endl;
    simple_index();
    
    return 0;
}