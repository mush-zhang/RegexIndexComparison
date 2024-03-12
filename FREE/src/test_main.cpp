#include "Index/plain_multigram.hpp"
#include "Index/presuf_shell.hpp"
#include <iostream> 
#include <cassert>

bool compare_pos_list(const std::vector<long> & a, const std::vector<long> & b) {
    if (a.size() != b.size()) return false;
    std::unordered_set<long> as(a.begin(), a.end());
    std::unordered_set<long> bs(b.begin(), b.end());
    if (as.size() != a.size() || bs.size() != b.size()) return false;
    return as == bs;
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

    auto pi = free_index::PlainMultigram(test_dataset, 1);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    assert(compare_pos_list(pi.get_line_pos_at("0"), {0}) && 
           compare_pos_list(pi.get_line_pos_at("1"), {1}) && 
           compare_pos_list(pi.get_line_pos_at("2"), {2}) && 
           compare_pos_list(pi.get_line_pos_at("3"), {3}) && 
           compare_pos_list(pi.get_line_pos_at("4"), {4}) &&
           compare_pos_list(pi.get_line_pos_at("5"), {5}) &&  
            "Line index should be in keys");

    assert(compare_pos_list(pi.get_line_pos_at("a"), {0}) && 
           compare_pos_list(pi.get_line_pos_at("b"), {1}) && 
           compare_pos_list(pi.get_line_pos_at("c"), {2}) && 
           compare_pos_list(pi.get_line_pos_at("d"), {3}) && 
           compare_pos_list(pi.get_line_pos_at("e"), {4}) &&
           compare_pos_list(pi.get_line_pos_at("f"), {5}) &&  
            "Alphabet char should have their respecting line idx and ony the one line idx");
    
    assert(compare_pos_list(pi.get_line_pos_at("."), {0, 1, 2, 3, 4, 5}) && 
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

    auto pi = free_index::PlainMultigram(test_dataset, 0.9);

    // try build index of uni and bigrams only
    pi.build_index(2);
    pi.print_index();

    assert(pi.get_line_pos_at(".").empty() && 
           "Char . should not be indexed due to selectivity threshold");
    assert(compare_pos_list(pi.get_line_pos_at(".a"), {0}) && 
           compare_pos_list(pi.get_line_pos_at(".b"), {1}) && 
           compare_pos_list(pi.get_line_pos_at(".c"), {2}) && 
           compare_pos_list(pi.get_line_pos_at(".d"), {3}) && 
           compare_pos_list(pi.get_line_pos_at(".e"), {4}) &&
           compare_pos_list(pi.get_line_pos_at(".f"), {5}) &&  
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

    auto pi = free_index::PresufShell(test_dataset, 0.9);

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
    assert(compare_pos_list(pi.get_line_pos_at("a"), {0}) && 
           compare_pos_list(pi.get_line_pos_at("b"), {1}) && 
           compare_pos_list(pi.get_line_pos_at("c"), {2}) && 
           compare_pos_list(pi.get_line_pos_at("d"), {3}) && 
           compare_pos_list(pi.get_line_pos_at("e"), {4}) &&
           compare_pos_list(pi.get_line_pos_at("f"), {5}) &&  
            "Alphabet char should have their respecting line idx and ony the one line idx");
    

}

int main() {

    simple_index();
    std::cout << "-------------------------------------------" << std::endl;
    simple_index_threshold();
    std::cout << "-------------------------------------------" << std::endl;
    simple_presuf();
    std::cout << "-------------------------------------------" << std::endl;

    return 0;
}