#include "simple_query_matcher.hpp"
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
    bool is_first = true;
    for (const auto & key : all_keys) {
        if (is_first) {
            idx_list = k_index_.get_line_pos_at(key);
            is_first = false;
        } else {
            auto curr_idxs = k_index_.get_line_pos_at(key);
            idx_list = sorted_lists_intersection(idx_list, curr_idxs);
        }
    }
    for (auto idx : idx_list) {
        count += RE2::PartialMatch(dataset[idx], *compiled_reg);
    }
    return count;
}

void SimpleQueryMatcher::match_all() {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<long> counts;
    counts.reserve(reg_evals_.size());

    for (const auto & [reg, compiled_reg] : reg_evals_) {
        counts.push_back(match_one_helper(reg, compiled_reg));
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Match All End in " << elapsed << " s" << std::endl;
    k_index_.write_to_file(std::to_string(elapsed) + "\n");

    for (long c : counts) {
        // std::cout << "[" << reg << "] : " << c << std::endl;
        std::cout << c << std::endl;
    }
}

void SimpleQueryMatcher::match_one(const std::string & reg) {
    auto start = std::chrono::high_resolution_clock::now();
    long count = 0;
    if (reg_evals_.find(reg) != reg_evals_.end()) {
        auto compiled_reg = reg_evals_[reg];
        long count = match_one_helper(reg, compiled_reg);
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Match One End in " << elapsed << " s" << std::endl;

    std::cout << "[" << reg << "] : " << count << std::endl;
}
