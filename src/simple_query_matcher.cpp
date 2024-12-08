#include "simple_query_matcher.hpp"
#include "utils/utils.hpp"

bool SimpleQueryMatcher::get_indexed(const std::string & reg,
                                     std::vector<size_t> & container) const {
    long count = 0;
    auto all_keys = k_index_.find_all_keys(reg);
    if (all_keys.empty()) {
        return false;
    }
    container.clear();
    bool is_first = true;
    for (const auto & key : all_keys) {
        if (is_first) {
            container = k_index_.get_line_pos_at(key);
            is_first = false;
        } else {
            auto curr_idxs = k_index_.get_line_pos_at(key);
            container = sorted_lists_intersection(container, curr_idxs);
        }
    }
    return true;
}

long SimpleQueryMatcher::match_one_helper(
        const std::string & reg, 
        const std::shared_ptr<RE2> compiled_reg) {
    auto dataset = k_index_.get_dataset();
    long count = 0;
    std::vector<size_t> idx_list;
    if (get_indexed(reg, idx_list)) {
        for (auto idx : idx_list) {
            count += RE2::PartialMatch(dataset[idx], *compiled_reg);
        }
    } else {
        for (const auto & l : dataset) {
            count += RE2::PartialMatch(l, *compiled_reg);
        }
    }

    return count;
}

std::vector<long> SimpleQueryMatcher::match_all() {
    if (reg_evals_.size() < k_index_.get_queries().size()) {
        compile_all_queries(k_index_.get_queries(), false);
    }
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
    
    return counts;
}

long SimpleQueryMatcher::match_one(const std::string & reg) {
    auto start = std::chrono::high_resolution_clock::now();
    if (reg_evals_.find(reg) == reg_evals_.end()) {
        reg_evals_[reg] = std::make_shared<RE2>(reg); 
    }
    auto compiled_reg = reg_evals_[reg];
    long count = match_one_helper(reg, compiled_reg);

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    // std::cout << "[" << reg << "] : " << count << std::endl;
    std::ostringstream log;
    log << elapsed << "\t" << count << "\t";
    k_index_.write_to_file(log.str());
    return count;
}

size_t SimpleQueryMatcher::get_num_after_filter(const std::string & reg) const {
    std::vector<size_t> idx_list;
    if (get_indexed(reg, idx_list)) {
        return idx_list.size();
    }
    return k_index_.get_dataset_size();;
}