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
                 : k_index_(index) {
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_shared<RE2>(reg_str);
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "Compile all End in " << elapsed << " s" << std::endl;
        k_index_.write_to_file(std::to_string(elapsed) + ",");
    }

    SimpleQueryMatcher(const NGramIndex & index) : k_index_(index) {
        auto start = std::chrono::high_resolution_clock::now();
        auto regs = k_index_.get_queries();
        assert(!regs.empty() &&
            "If the index has no regexes, pass in the regex set as second parameter");
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_shared<RE2>(reg_str);
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "Compile all End in " << elapsed << " s" << std::endl;
        k_index_.write_to_file(std::to_string(elapsed) + ",");
    }

    void match_all();

    void match_one(const std::string & reg);

    ~SimpleQueryMatcher() {}

 private:
    const NGramIndex & k_index_;

    std::unordered_map<std::string, std::shared_ptr<RE2>> reg_evals_;

    long match_one_helper(const std::string & reg, 
        const std::shared_ptr<RE2> compiled_reg);
};

#endif // SIMPLE_QUERY_MATCHER_HPP_