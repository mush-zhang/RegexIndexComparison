#include "presuf_shell.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>

// Section 3.2 Observation 3.13 proof
void free::PresufShell::build_index(int upper_k) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_k);
    compute_suffix_free_set();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << elapsed << " s" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    fill_posting(upper_k);
    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}

// Reverse the strings in the prefix free set X identified by 
//   algorithm 3.1 and then sort them in lexicographic order.
void free::PresufShell::compute_suffix_free_set() {
    if (k_index_keys_.size() < 2) return;

    std::unordered_map<std::string, std::string> rev_to_ori;
    std::vector<std::string> temp_rev_list;

    for (const auto & key : k_index_keys_) {
        std::string rev_key(key);
        std::reverse(rev_key.begin(), rev_key.end());
        temp_rev_list.push_back(rev_key);
        rev_to_ori[rev_key] = key;
    }

    std::sort(temp_rev_list.begin(), temp_rev_list.end());

    std::string & curr_prefix = temp_rev_list[0];
    for (size_t i = 0; i < temp_rev_list.size(); i++) {
        auto curr_full = temp_rev_list[i];
        if (curr_full.size() > curr_prefix.size() && 
            curr_full.substr(0, curr_prefix.size()) == curr_prefix) {
                // remove this string
                k_index_keys_.erase(rev_to_ori[curr_full]);
            }
            curr_prefix = curr_full;
    }
}
