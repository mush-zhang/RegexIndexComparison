#include "Index/plain_multigram.hpp"
// #include<iostream> 

int main(int argc, char** argv) {
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

    return 0;
}