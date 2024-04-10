#include "Index/lpms.hpp"

#include "../simple_query_matcher.hpp"

#include <iostream> 
#include <cassert>
#include <unordered_set>

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
    std::vector<std::string> test_dataset({
        "an",
        "York",
        "Franc",
        "kane",
    });;

    auto pi = fast_index::LpmsIndex(test_dataset, test_query);

    pi.build_index();
    pi.print_index();
}

void simple_index_radonmized() {
    std::vector<std::string> test_query({
        "an",
        "York",
        "Franc",
        "kane",
    });
    std::vector<std::string> test_dataset({
        "an",
        "York",
        "Franc",
        "kane",
    });;

    auto pi2 = fast_index::LpmsIndex(test_dataset, test_query, 
        fast_index::LpmsIndex::relaxation_type::kRandomized);
    pi2.build_index();
    pi2.print_index();
    std::cout << "***********" << std::endl;
}

void simple_matcher() {
    std::vector<std::string> test_query({
        "ane",
        "ork",
        "ka"
    });;

    std::vector<std::string> test_dataset;
    double threshold;
    make_dataset_with_keys(test_query, test_dataset, threshold);

    auto pi = fast_index::LpmsIndex(test_dataset, test_query);
    pi.build_index();
    pi.print_index();
    auto matcher = SimpleQueryMatcher(pi);
    matcher.match_all();
}

int main() {
    std::cout << "BEGIN INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "\t SIMPLE LP 0.99 -------------------------------------------" << std::endl;
    simple_index();
    std::cout << "\t END SIMPLE LP 0.99 -------------------------------------------" << std::endl;
    std::cout << "\t SIMPLE RANDOMIZED -------------------------------------------" << std::endl;
    simple_index_radonmized();    
    std::cout << "\t END SIMPLE RANDOMIZED -------------------------------------------" << std::endl;
    std::cout << "\t MATCHER -------------------------------------------" << std::endl;
    simple_matcher();    
    std::cout << "\t END MATCHER -------------------------------------------" << std::endl;
    return 0;
}