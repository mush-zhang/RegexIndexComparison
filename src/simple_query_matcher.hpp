#ifndef SIMPLE_QUERY_MATCHER_HPP_
#define SIMPLE_QUERY_MATCHER_HPP_

#include <memory>
#include <re2/re2.h>
#include <cassert>

#include "ngram_index.hpp"

class SimpleQueryMatcher {
 public:
    SimpleQueryMatcher() = delete;

    SimpleQueryMatcher(const NGramIndex & index, 
                 const std::vector<std::string> & regs) 
                 : index_(index) {
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_shared<RE2>(reg_str);
        }
    }

    SimpleQueryMatcher(const NGramIndex & index) : index_(index) {
        auto regs = index_.get_queries();
        assert(!regs.empty() &&
            "If the index has no regexes, pass in the regex set as second parameter");
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_shared<RE2>(reg_str);
        }
    }

    void match_all();

    void match_one(const std::string & reg);

    ~SimpleQueryMatcher() {}

 private:
    const NGramIndex & index_;

    std::unordered_map<std::string, std::shared_ptr<RE2>> reg_evals_;

    long match_one_helper(const std::string & reg, 
        const std::shared_ptr<RE2> compiled_reg);
};

#endif // SIMPLE_QUERY_MATCHER_HPP_