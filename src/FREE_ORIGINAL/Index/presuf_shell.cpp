#include "presuf_shell.hpp"
#include <algorithm>

// Section 3.2 Observation 3.13 proof
void free_original_index::PresufShell::build_index(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_n);
    compute_suffix_free_set();
    auto selection_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << selection_time << " s" << std::endl;

    std::ostringstream log;    
    log << "FREE" << k_tag_ << "," << thread_count_ << "," << upper_n << ",";
    log << k_threshold_ << ","  << key_upper_bound_ << "," << k_queries_size_ << ",";
    log << selection_time << ",";

    start = std::chrono::high_resolution_clock::now();
    fill_posting(upper_n);
    auto build_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << build_time << std::endl;
    
    log << build_time << "," << build_time+selection_time << ",";
    log << get_num_keys() << "," << get_bytes_used() << ",";
    write_to_file(log.str());
}

// Reverse the strings in the prefix free set X identified by 
//   algorithm 3.1 and then sort them in lexicographic order.
void free_original_index::PresufShell::compute_suffix_free_set() {
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
