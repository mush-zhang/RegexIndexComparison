#include "simple_query_matcher.hpp"
#include <iostream>
#include "utils/utils.hpp"

long SimpleQueryMatcher::match_one_helper(
        const std::string & reg, 
        const std::shared_ptr<RE2> compiled_reg) {
    auto dataset = k_index_.get_dataset();
    long count = 0;
    auto all_keys = k_index_.find_all_indexed(reg);
    if (all_keys.empty()) {
        for (const auto & l : dataset) {
            count += RE2::PartialMatch(l, *compiled_reg);
        }
        return count;
    }
    std::vector<size_t> idx_list;
    for (const auto & key : all_keys) {
        auto curr_idxs = k_index_.get_line_pos_at(key);
        idx_list = sorted_lists_intersection(idx_list, curr_idxs);
    }
    for (auto idx : idx_list) {
        count += RE2::PartialMatch(dataset[idx], *compiled_reg);
    }
    return count;
}

void SimpleQueryMatcher::match_all() {
    for (const auto & [reg, compiled_reg] : reg_evals_) {
        long count = match_one_helper(reg, compiled_reg);
        std::cout << count << std::endl;
    }
    // TOOD: add timing print maybe.
}

void SimpleQueryMatcher::match_one(const std::string & reg) {
    if (reg_evals_.find(reg) != reg_evals_.end()) {
        auto compiled_reg = reg_evals_[reg];
        long count = match_one_helper(reg, compiled_reg);
        std::cout << count << std::endl;
    }
}
