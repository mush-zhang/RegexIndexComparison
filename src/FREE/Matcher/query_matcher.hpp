#ifndef FREE_MATCHER_QUERY_MATCHER_HPP_
#define FREE_MATCHER_QUERY_MATCHER_HPP_

#include <iostream>
#include <re2/re2.h>
#include "query_parser.hpp"
#include "../Index/multigram_index.hpp"
#include "../Index/presuf_shell.hpp"

namespace free_index {

class QueryMatcher {
 public:
    QueryMatcher() = delete;
    
    QueryMatcher(const MultigramIndex & index, 
                 const std::vector<std::string> & regs) 
                 : k_index_(index) {
        for (const auto & reg : regs) {
            add_query(reg);
        }
    }

    void match_all() {
        for (const auto & [reg_str, comps] : reg_evals_) {
            long count = match_one_helper(comps.first, comps.second);
            std::cout << "[" << reg_str << "] : " << count << std::endl;
        }
        // TOOD: add timing print maybe.
    }

    void match_one(const std::string & reg) {
        if (reg_evals_.find(reg) == reg_evals_.end()) {
            add_query(reg);
        } 
        const auto & [parser, regex] = reg_evals_[reg];
        long count = match_one_helper(parser, regex);
        std::cout << "[" << reg << "] : " << count << std::endl;
    }

    ~QueryMatcher() {}

 private:
    const MultigramIndex & k_index_;

    std::unordered_map<std::string, 
                       std::pair<std::unique_ptr<QueryParser>, std::shared_ptr<RE2>>> reg_evals_;

    long match_one_helper(const std::unique_ptr<QueryParser> & parser, const std::shared_ptr<RE2> & regex) {
        auto dataset = k_index_.get_dataset();
        long count = 0;
        std::vector<size_t> idx_list;
        bool helped = parser->get_index_by_plan(idx_list);
        if (helped) {
            for (auto idx : idx_list) {
                count += RE2::PartialMatch(dataset[idx], *regex);
            }
        } else {
            for (const auto & line : dataset) {
                count += RE2::PartialMatch(line, *regex);
            }
        }
        return count;
    }

    void add_query(const std::string & reg) {
        auto curr_p = std::make_unique<QueryParser>(k_index_);
        curr_p->generate_query_plan(reg);
        curr_p->remove_null();
        curr_p->rewrite_by_index();
        curr_p->remove_null();
        reg_evals_[reg] = std::make_pair(std::move(curr_p), std::make_shared<RE2>(reg));
    }
};

} // namespace free_index

#endif // FREE_MATCHER_QUERY_MATCHER_HPP_