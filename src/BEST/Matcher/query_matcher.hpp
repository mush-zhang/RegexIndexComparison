#ifndef BEST_MATCHER_QUERY_MATCHER_HPP_
#define BEST_MATCHER_QUERY_MATCHER_HPP_

#include <memory>
#include <re2/re2.h>
#include "../ngram_btree_index.hpp"

namespace best_index {

class QueryMatcher {
 public:
    QueryMatcher() {};

    QueryMatcher(NGramBtreeIndex * index, 
                 const std::vector<std::string> & regs) 
                 : k_dataset_(index->get_dataset()), index_(index) {
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_unique<RE2>(reg_str);
        }
    }

    void match_all() {
        for (const auto & [reg_str, compiled_reg] : k_reg_strings_) {
            long count = 0;
            auto idx_list = index.find_all_indexed(reg_str);
            for (auto idx : idx_list) {
                count += RE2::PartialMatch(k_dataset_[idx], compiled_reg);
            }
            std::cout << count << std::endl;
        }
        // TOOD: add timing print maybe.
    }

    void match_one(const std::string & reg) {
        if (reg_evals_.find(reg) != reg_evals_.end()) {
            const auto & compiled_reg = reg_evals_[reg];
            long count = 0;
            auto idx_list = index.find_all_indexed(reg_str);
            for (auto idx : idx_list) {
                count += RE2::PartialMatch(k_dataset_[idx], compiled_reg);
            }
            std::cout << count << std::endl;
        }
    }

    ~QueryMatcher() {}

 private:
    const std::vector<std::string> & k_dataset_;
    std::shared_ptr<NGramBtreeIndex> index_;

    std::unordered_map<std::string, std::unique_ptr<RE2>> reg_evals_;
};

} // namespace best_index

#endif // BEST_MATCHER_QUERY_MATCHER_HPP_