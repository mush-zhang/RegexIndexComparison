#include "Index/single_threaded.hpp"
#include "Index/parallelizable.hpp"

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

    auto pi1 = best_index::SingleThreadedIndex(test_dataset, test_query, 
        1, 4, best_index::SingleThreadedIndex::dist_type::kMaxDevDist1);
    pi1.build_index();
    pi1.print_index();
    std::cout << "***********" << std::endl;

    auto pi2 = best_index::SingleThreadedIndex(test_dataset, test_query, 
        1, 4, best_index::SingleThreadedIndex::dist_type::kMaxDevDist2);
    pi2.build_index();
    pi2.print_index();
    std::cout << "***********" << std::endl;

    auto pi3 = best_index::SingleThreadedIndex(test_dataset, test_query, 
        1, 4, best_index::SingleThreadedIndex::dist_type::kMaxDevDist3);
    pi3.build_index();
    pi3.print_index();
    std::cout << "***********" << std::endl;

}

void simple_parallelizable() {
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

    auto pi = best_index::ParallelizableIndex(test_dataset, test_query, 1, 4, 
        best_index::SingleThreadedIndex::dist_type::kMaxDevDist1);
    pi.build_index();
    pi.print_index();
}

void simple_matcher() {
    std::vector<std::string> test_query({
        "ane", // [4, 9]
        "ork", // [2]
        "ka" // [4, 8, 12]
    });;
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

    auto pi = best_index::SingleThreadedIndex(test_dataset, test_query, 1, 4, 
        best_index::SingleThreadedIndex::dist_type::kMaxDevDist1);
    pi.build_index();
    pi.print_index();
    auto matcher = SimpleQueryMatcher(pi);
    std::cout << "\t match all" << std::endl;
    matcher.match_all();
    std::cout << "\t match one" << std::endl;
    matcher.match_one(test_query[1]);
}

void longer_matcher() {
    std::vector<std::string> test_query({
        "ane",
        "ork",
        "ka"
    });;

    std::vector<std::string> test_dataset;
    double threshold;
    make_dataset_with_keys(test_query, test_dataset, threshold);

    auto pi = best_index::SingleThreadedIndex(test_dataset, test_query, 1, 4, 
        best_index::SingleThreadedIndex::dist_type::kMaxDevDist1);
    pi.build_index();
    pi.print_index();
    auto matcher = SimpleQueryMatcher(pi);
    std::cout << "\t match all" << std::endl;
    matcher.match_all();
    std::cout << "\t match one" << std::endl;
    matcher.match_one(test_query[0]);
}

int main() {
    std::cout << "BEGIN INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "\t GRAM CANDIDATE SET -------------------------------------------" << std::endl;
    simple_index();
    std::cout << "\t END GRAM CANDIDATE SET -------------------------------------------" << std::endl;
    std::cout << "\t WORKLOAD REDUCTION -------------------------------------------" << std::endl;
    simple_wl_red_index();    
    std::cout << "\t END WORKLOAD REDUCTION -------------------------------------------" << std::endl;
    std::cout << "\t PARALLEL INDEX -------------------------------------------" << std::endl;
    simple_parallelizable();    
    std::cout << "\t END PARALLEL INDEX -------------------------------------------" << std::endl;
    std::cout << "END INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "BEGIN MATCHER TESTS -------------------------------------------" << std::endl;
    std::cout << "\t SIMPLE MATCHER -------------------------------------------" << std::endl;
    simple_matcher();    
    std::cout << "\t END SIMPLE MATCHER -------------------------------------------" << std::endl;
    std::cout << "\t LONGER MATCHER -------------------------------------------" << std::endl;
    longer_matcher();    
    std::cout << "\t END LONGER MATCHER -------------------------------------------" << std::endl;
    std::cout << "END MATCHER TESTS -------------------------------------------" << std::endl;
    return 0;
}