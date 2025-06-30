#ifndef SIMPLE_QUERY_MATCHER_HPP_
#define SIMPLE_QUERY_MATCHER_HPP_

#include <memory>
#include <re2/re2.h>
#include <cassert>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>

#include "ngram_index.hpp"

class SimpleQueryMatcher {
 public:
    SimpleQueryMatcher() = delete;

    SimpleQueryMatcher(const NGramIndex & index, 
                 const std::vector<std::string> & regs,
                 bool compile=true) 
                 : k_index_(index) {
        if (compile) {
            compile_all_queries(regs);
        }
    }

    SimpleQueryMatcher(const NGramIndex & index, bool compile=true) : k_index_(index) {
        auto regs = k_index_.get_queries();
        assert(!regs.empty() &&
            "If the index has no regexes, pass in the regex set as second parameter");
        if (compile) {
            compile_all_queries(regs);
        }
    }

    std::vector<long> match_all();

    long match_one(const std::string & reg);

    // Parallel version of individual stats collection
    void collect_parallel_stats(const std::vector<std::string> & regexes, 
                               int num_threads = 0);

    // Parallel version for baseline (writes -1 for filter count)
    void collect_parallel_stats_baseline(const std::vector<std::string> & regexes, 
                                        int num_threads = 0);

    size_t get_num_after_filter(const std::string & reg) const;

    ~SimpleQueryMatcher() {}

 protected:
    const NGramIndex & k_index_;

    std::unordered_map<std::string, std::shared_ptr<RE2>> reg_evals_;

    virtual bool get_indexed(const std::string & reg, std::vector<size_t> & container) const;

    long match_one_helper(const std::string & reg, const std::shared_ptr<RE2> compiled_reg);

    // Thread-safe version of match_one for parallel processing
    void match_one_parallel(const std::string & reg, size_t index);

    // Thread-safe version for baseline (writes -1 for filter count)
    void match_one_parallel_baseline(const std::string & reg, size_t index);

    // Worker function for parallel stats collection
    void parallel_worker(const std::vector<std::string> & regexes,
                         size_t start_idx, size_t end_idx);

    // Worker function for baseline parallel stats collection
    void parallel_worker_baseline(const std::vector<std::string> & regexes,
                                 size_t start_idx, size_t end_idx);

    void compile_all_queries(const std::vector<std::string> & regs, bool log=true) {
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto & reg_str : regs) {
            reg_evals_[reg_str] = std::make_shared<RE2>(reg_str);
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
        std::cout << "Compile all End in " << elapsed << " s" << std::endl;
        if (log)
            k_index_.write_to_file(std::to_string(elapsed) + ",");
    }
};

#endif // SIMPLE_QUERY_MATCHER_HPP_