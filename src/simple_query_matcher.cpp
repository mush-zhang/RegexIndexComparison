#include "simple_query_matcher.hpp"
#include "utils/utils.hpp"
#include <algorithm>
#include <thread>

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
    if (reg_evals_.empty()) {
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
    return k_index_.get_dataset_size();
}

void SimpleQueryMatcher::match_one_parallel(const std::string & reg, size_t index) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Get or compile the regex (thread-safe since we're only reading)
    std::shared_ptr<RE2> compiled_reg;
    if (reg_evals_.find(reg) == reg_evals_.end()) {
        compiled_reg = std::make_shared<RE2>(reg);
    } else {
        compiled_reg = reg_evals_[reg];
    }
    
    long count = match_one_helper(reg, compiled_reg);
    size_t num_after_filter = get_num_after_filter(reg);

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    
    // Thread-safe write to file
    std::ostringstream log;
    log << reg << "\t" << elapsed << "\t" << count << "\t" << num_after_filter;
    k_index_.write_to_file_line(log.str());
}

void SimpleQueryMatcher::parallel_worker(const std::vector<std::string> & regexes,
                                        size_t start_idx, size_t end_idx) {
    for (size_t i = start_idx; i < end_idx; ++i) {
        match_one_parallel(regexes[i], i);
    }
}

void SimpleQueryMatcher::collect_parallel_stats(const std::vector<std::string> & regexes, 
                                               int num_threads) {
    if (num_threads <= 0) {
        num_threads = std::max(1u, std::thread::hardware_concurrency());
    }
    
    const size_t total_regexes = regexes.size();
    const size_t regexes_per_thread = (total_regexes + num_threads - 1) / num_threads;
    
    std::vector<std::thread> workers;
    workers.reserve(num_threads);
    
    std::cout << "Starting parallel stats collection with " << num_threads 
              << " threads for " << total_regexes << " regexes..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Launch worker threads
    for (int t = 0; t < num_threads; ++t) {
        size_t start_idx = t * regexes_per_thread;
        size_t end_idx = std::min(start_idx + regexes_per_thread, total_regexes);
        
        if (start_idx < total_regexes) {
            workers.emplace_back(&SimpleQueryMatcher::parallel_worker, this,
                               std::cref(regexes), start_idx, end_idx);
        }
    }
    
    // Wait for all threads to complete
    for (auto & worker : workers) {
        worker.join();
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start_time).count();
    std::cout << "Parallel stats collection completed in " << elapsed << " seconds" << std::endl;
}

void SimpleQueryMatcher::match_one_parallel_baseline(const std::string & reg, size_t index) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Get or compile the regex (thread-safe since we're only reading)
    std::shared_ptr<RE2> compiled_reg;
    if (reg_evals_.find(reg) == reg_evals_.end()) {
        compiled_reg = std::make_shared<RE2>(reg);
    } else {
        compiled_reg = reg_evals_[reg];
    }
    
    long count = match_one_helper(reg, compiled_reg);

    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    
    // Thread-safe write to file (baseline uses -1 for filter count)
    std::ostringstream log;
    log << reg << "\t" << elapsed << "\t" << count << "\t" << "-1";
    k_index_.write_to_file_line(log.str());
}

void SimpleQueryMatcher::parallel_worker_baseline(const std::vector<std::string> & regexes,
                                                  size_t start_idx, size_t end_idx) {
    for (size_t i = start_idx; i < end_idx; ++i) {
        match_one_parallel_baseline(regexes[i], i);
    }
}

void SimpleQueryMatcher::collect_parallel_stats_baseline(const std::vector<std::string> & regexes, 
                                                        int num_threads) {
    if (num_threads <= 0) {
        num_threads = std::max(1u, std::thread::hardware_concurrency());
    }
    
    const size_t total_regexes = regexes.size();
    const size_t regexes_per_thread = (total_regexes + num_threads - 1) / num_threads;
    
    std::vector<std::thread> workers;
    workers.reserve(num_threads);
    
    std::cout << "Starting parallel baseline stats collection with " << num_threads 
              << " threads for " << total_regexes << " regexes..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Launch worker threads
    for (int t = 0; t < num_threads; ++t) {
        size_t start_idx = t * regexes_per_thread;
        size_t end_idx = std::min(start_idx + regexes_per_thread, total_regexes);
        
        if (start_idx < total_regexes) {
            workers.emplace_back(&SimpleQueryMatcher::parallel_worker_baseline, this,
                               std::cref(regexes), start_idx, end_idx);
        }
    }
    
    // Wait for all threads to complete
    for (auto & worker : workers) {
        worker.join();
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start_time).count();
    
    std::cout << "Parallel baseline stats collection completed in " << elapsed << " seconds" << std::endl;
}