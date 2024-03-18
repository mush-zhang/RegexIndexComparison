#include "Index/multigram_index.hpp"
#include "Index/presuf_shell.hpp"
#include "Matcher/query_parser.hpp"

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

void make_dataset_with_keys(const std::vector<std::string> & keys, std::vector<std::string> & dataset_container, double & threshold_constainer) {
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
    std::vector<std::string> test_dataset({
        "0.aaaaa",
        "1.bbbbb",
        "2.ccccc",
        "3.ddddd",
        "4.eeeee",
        "5.fffff"
    });

    auto pi = free::MultigramIndex(test_dataset, 1);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    assert(compare_lists(pi.get_line_pos_at("0"), {0}) && 
           compare_lists(pi.get_line_pos_at("1"), {1}) && 
           compare_lists(pi.get_line_pos_at("2"), {2}) && 
           compare_lists(pi.get_line_pos_at("3"), {3}) && 
           compare_lists(pi.get_line_pos_at("4"), {4}) &&
           compare_lists(pi.get_line_pos_at("5"), {5}) &&  
            "Line index should be in keys");

    assert(compare_lists(pi.get_line_pos_at("a"), {0}) && 
           compare_lists(pi.get_line_pos_at("b"), {1}) && 
           compare_lists(pi.get_line_pos_at("c"), {2}) && 
           compare_lists(pi.get_line_pos_at("d"), {3}) && 
           compare_lists(pi.get_line_pos_at("e"), {4}) &&
           compare_lists(pi.get_line_pos_at("f"), {5}) &&  
            "Alphabet char should have their respecting line idx and ony the one line idx");
    
    assert(compare_lists(pi.get_line_pos_at("."), {0, 1, 2, 3, 4, 5}) && 
           "Char . in all 0-5 lines");
}

void simple_index_threshold() {
    std::vector<std::string> test_dataset({
        "0.aaaaa",
        "1.bbbbb",
        "2.ccccc",
        "3.ddddd",
        "4.eeeee",
        "5.fffff"
    });

    auto pi = free::MultigramIndex(test_dataset, 0.9);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    assert(pi.get_line_pos_at(".").empty() && 
           "Char . should not be indexed due to selectivity threshold");
    assert(compare_lists(pi.get_line_pos_at(".a"), {0}) && 
           compare_lists(pi.get_line_pos_at(".b"), {1}) && 
           compare_lists(pi.get_line_pos_at(".c"), {2}) && 
           compare_lists(pi.get_line_pos_at(".d"), {3}) && 
           compare_lists(pi.get_line_pos_at(".e"), {4}) &&
           compare_lists(pi.get_line_pos_at(".f"), {5}) &&  
            "Dot+alphabet should have their respecting line idx and ony the one line idx");
    assert(pi.get_line_pos_at("0.").empty() && 
           pi.get_line_pos_at("1.").empty() && 
           pi.get_line_pos_at("2.").empty() && 
           pi.get_line_pos_at("3.").empty() && 
           pi.get_line_pos_at("4.").empty() &&
           pi.get_line_pos_at("5.").empty() &&  
            "Dot+alphabet should have been removed due to prefix free");
}

void simple_presuf() {
    std::vector<std::string> test_dataset({
        "0.aaaaa",
        "1.bbbbb",
        "2.ccccc",
        "3.ddddd",
        "4.eeeee",
        "5.fffff"
    });

    auto pi = free::PresufShell(test_dataset, 0.9);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    assert(pi.get_line_pos_at(".").empty() && 
           "Char . should not be indexed due to selectivity threshold");
    assert(pi.get_line_pos_at(".a").empty() && 
           pi.get_line_pos_at(".b").empty() && 
           pi.get_line_pos_at(".c").empty() && 
           pi.get_line_pos_at(".d").empty() && 
           pi.get_line_pos_at(".e").empty() &&
           pi.get_line_pos_at(".f").empty() &&  
            "Dot+alphabet should have been removed due to suffix free");
    assert(compare_lists(pi.get_line_pos_at("a"), {0}) && 
           compare_lists(pi.get_line_pos_at("b"), {1}) && 
           compare_lists(pi.get_line_pos_at("c"), {2}) && 
           compare_lists(pi.get_line_pos_at("d"), {3}) && 
           compare_lists(pi.get_line_pos_at("e"), {4}) &&
           compare_lists(pi.get_line_pos_at("f"), {5}) &&  
            "Alphabet char should have their respecting line idx and ony the one line idx");
}

void simple_find_keys() {
    std::vector<std::string> test_keys({
        "Will",
        "liam",
        "Clint",
        "nton"
    });

    std::vector<std::string> test_dataset;
    double threshold;
    make_dataset_with_keys(test_keys, test_dataset, threshold);

    auto pi = free::MultigramIndex(test_dataset, threshold);
    pi.build_index(5);
    pi.print_index();

    assert(pi.find_all_indexed("Bill").empty() && "Bill not in index");
    assert(compare_lists(pi.find_all_indexed("William"), {"Will", "liam"}) && 
           "2 Keys indexed in William");
    assert(compare_lists(pi.find_all_indexed("Clinton"), {"Clint", "nton"}) && 
           "2 Keys indexed in Clinton");
}

void simple_query_parser() {
    std::string reg_query = "(Bill|William)(.*)Clinton";
    
    auto qp = free::QueryParser();

    // Should be same as Figure 6(b)
    qp.generate_query_plan(reg_query);
    qp.print_plan();

    // Should be same as Figure 6(c)
    qp.remove_null();
    qp.print_plan();
}

void simple_query_plan_by_index() {
    std::vector<std::string> test_keys({
        "Will",
        "liam",
        "Clint",
        "nton"
    });

    std::vector<std::string> test_dataset;
    double threshold;
    make_dataset_with_keys(test_keys, test_dataset, threshold);

    auto pi = free::MultigramIndex(test_dataset, threshold);
    pi.build_index(5);
    
    std::string reg_query = "(Bill|William)(.*)Clinton";
    auto qp = free::QueryParser(&pi);

    qp.generate_query_plan(reg_query);
    // Should be same as Figure 7(a)
    qp.remove_null();
    qp.rewrite_by_index();
    qp.print_plan();

    // Should be same as Figure 7(b)
    qp.remove_null();
    qp.print_plan();
}

void simple_lines_by_plan() {
    std::vector<std::string> test_keys({
        "Will",
        "liam",
        "Clint",
        "nton"
    });

    std::vector<std::string> test_dataset;
    double threshold;
    make_dataset_with_keys(test_keys, test_dataset, threshold);
    threshold = 4.0/(42.0+test_dataset.size());
    for (size_t i = 0; i < k_number_repeat; i++) {
        test_dataset.push_back("linto");
        test_dataset.push_back("illi");
        test_dataset.push_back("ton");
        test_dataset.push_back("llia");
    }
    test_dataset.push_back("William");
    test_dataset.push_back("Clinton");
    auto pi = free::PresufShell(test_dataset, threshold);
    pi.build_index(5);
    pi.print_index();
    std::string reg_query = "(Bill|William)(.*)Clinton";
    auto qp = free::QueryParser(&pi);

    qp.generate_query_plan(reg_query);
    qp.rewrite_by_index();
    // Should be same as Figure 7(b)
    qp.remove_null();
    qp.print_plan();

    auto idx_list = qp.get_index_by_plan();
    assert(compare_lists(idx_list, {455}) && 
           "Line 455 Clinton should be the only result");
}

int main() {
    std::cout << "BEGIN INDEX TESTS -------------------------------------------" << std::endl;
    std::cout << "\t SIMPLE INDEX-------------------------------------------" << std::endl;
    simple_index();
    std::cout << "\t SIMPLE INDEX THRESHOLD-------------------------------------------" << std::endl;
    simple_index_threshold();
    std::cout << "\t SIMPLE PRESUF-------------------------------------------" << std::endl;
    simple_presuf();
    std::cout << "\t SIMPLE FIND KEYS-------------------------------------------" << std::endl;
    simple_find_keys();
    std::cout << "BEGIN PARSER TESTS-------------------------------------------" << std::endl;
    std::cout << "\t SIMPLE QUERY PARSER-------------------------------------------" << std::endl;
    simple_query_parser();    
    std::cout << "\t SIMPLE QUERY PLAN BY INDEX-------------------------------------------" << std::endl;
    simple_query_plan_by_index();
    std::cout << "\t SIMPLE LINES BY PLAN-------------------------------------------" << std::endl;
    simple_lines_by_plan();
    return 0;
}