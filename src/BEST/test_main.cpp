#include "Index/single_threaded.hpp"

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

    auto pi = best_index::SingleThreadedIndex(test_dataset, test_query, 1);

    pi.build_index();
    pi.print_index();
}

void simple_wl_red_index() {
    std::vector<std::string> test_query({
        "an", //1
        "York", //2
        "Franc", //3
        "kane", //4
        "anc", //5
        "Yor", //6
        "Fran", //7
        "kan", //8
        "ane", //9
        "Yo", //10
        "Fra", //11
        "ka" //12
    });
    std::vector<std::string> test_dataset({
        "an", //1
        "York", //2
        "Franc", //3
        "kane", //4
        "anc", //5
        "Yor", //6
        "Fran", //7
        "kan", //8
        "ane", //9
        "Yo", //10
        "Fra", //11
        "ka" //12
    });;

    auto pi = best_index::SingleThreadedIndex(test_dataset, test_query, 1, 4);

    pi.build_index();
    pi.print_index();
}

int main() {
    std::cout << "BEGIN INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "\t GRAM CANDIDATE SET -------------------------------------------" << std::endl;
    simple_index();
    std::cout << "\t END GRAM CANDIDATE SET -------------------------------------------" << std::endl;
    std::cout << "\t WORKLOAD REDUCTION -------------------------------------------" << std::endl;
    simple_wl_red_index();    
    std::cout << "\t END WORKLOAD REDUCTION -------------------------------------------" << std::endl;
    return 0;
}