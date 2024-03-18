#ifndef FREE_MATCHER_QUERY_MATCHER_HPP_
#define FREE_MATCHER_QUERY_MATCHER_HPP_

#include <memory>
#include <re2/re2.h>
#include "query_parser.hpp"
#include "../Index/multigram_index.hpp"
#include "../Index/presuf_shell.hpp"

namespace free_matcher {

class QueryMatcher {
 public:
    QueryMatcher() {};
    QueryMatcher(const std::vector<std::string> & dataset, double sel_threshold): k_dataset_(dataset) {
        index_ = std::make_shared<free_index::MultigramIndex>(dataset, sel_threshold);
        index_->build_index();
    }

    QueryMatcher(const std::vector<std::string> & dataset, double sel_threshold, 
                 const std::vector<std::string> & regs) 
                 : QueryMatcher(dataset, sel_threshold), k_reg_strings_(regs) {}
    
    QueryMatcher(free_index::MultigramIndex * index, 
                 const std::vector<std::string> & regs) 
                 : k_dataset_(index->get_dataset()), k_reg_strings_(regs), index_(index) {
        for (const auto & reg : k_regexes) {
            auto curr_p = std::make_unique<QueryParser>(index_);
            curr_p->generate_query_plan(reg);
            curr_p->remove_null();
            curr_p->rewrite_by_index();
            curr_p->remove_null();
            reg_evals_[reg] = std::make_pair(std::move(curr_p), std::make_unique<RE2>(reg))
        }

    }

    void match_all() {
        for (const auto & [parser, regex] : reg_evals_) {
            long count = 0;
            auto idx_list = parser.get_index_by_plan();
            for (auto idx : idx_list) {
                count += RE2::PartialMatch(k_dataset_[idx], regex);
            }
            std::cout << count << std::endl;
        }
        // TOOD: add timing print maybe.
    }

    void match_one(const std::string & reg) {
        if (reg_evals_.find(reg) != reg_evals_.end()) {
            const auto & [parser, regex] = reg_evals_[reg];
            long count = 0;
            auto idx_list = parser.get_index_by_plan();
            for (auto idx : idx_list) {
                count += RE2::PartialMatch(k_dataset_[idx], regex);
            }
            std::cout << count << std::endl;
        }
    }

    ~QueryMatcher() {}

    

 private:
    const std::vector<std::string> & k_dataset_;
    const std::vector<std::string> & k_reg_strings_;
    std::shared_ptr<free_index::MultigramIndex> index_;

    std::unordered_map<std::string, 
                       std::pair<std::unique_ptr<QueryParser>, std::unique_ptr<RE2>> reg_evals_;
};

} // namespace free_matcher

#endif // FREE_MATCHER_QUERY_MATCHER_HPP_