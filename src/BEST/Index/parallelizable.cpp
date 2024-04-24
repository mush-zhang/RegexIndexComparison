#include <thread>
#include <future>
#include <sstream>

#include "parallelizable.hpp"
#include "../../utils/utils.hpp"

void best_index::ParallelizableIndex::build_qg_list_local(
        std::vector<std::set<size_t>> & qg_list,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals,
        const std::vector<size_t> & q_list) {
    qg_list.assign(q_list.size(), std::set<size_t>());
    for (size_t i = 0; i < q_list.size(); i++) {
        auto q_idx = q_list[i];
        const auto & literals = query_literals[q_idx];
        indexed_grams_in_literals(literals, candidates, qg_list, i);
    }
}

void best_index::ParallelizableIndex::build_job_local(
        best_index::SingleThreadedIndex::job & job,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals,
        const std::vector<size_t> q_list) {
    build_qg_list_local(job.qg_list, candidates, query_literals, q_list);

    std::vector<bool> candidates_filter(candidates.size(), false);
    for (const auto & g_list : job.qg_list) {
        for (const auto & g_idx : g_list) {
            candidates_filter[g_idx] = true;
        }
    }
    std::vector<std::set<size_t>> rg_list(k_dataset_size_);
    for (size_t i = 0; i < k_dataset_size_; i++) {
        indexed_grams_in_string(k_dataset_[i], candidates, rg_list, i, candidates_filter);
    }
    build_gr_list_rc(job, candidates.size(), k_dataset_size_, rg_list);
}

// TODO: add stop token to notify all asyncs once got a false
//       https://www.geeksforgeeks.org/cpp-20-stop_token-header/
bool best_index::ParallelizableIndex::multi_all_covered(
        const std::set<size_t> & index, 
        const std::vector<best_index::SingleThreadedIndex::job> & jobs) {
    std::vector<std::future<bool>> futures;
    for (int i = 0; i < jobs.size(); i++) {
        futures.push_back(std::async(std::launch::async, 
            &best_index::ParallelizableIndex::all_covered, this,
                std::cref(index), std::cref(jobs[i]), jobs[i].qg_list.size()
        ));
    }
    for (auto & future_ret : futures) {
        if (!future_ret.get()) {
            return false;
        }
    }
    return true;
}

// Algorithm 4 in Figure 5
// Parallelizable greedy gram selection algorightm
void best_index::ParallelizableIndex::select_grams(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();

    auto query_literals = get_query_literals();
    auto pre_suf_count = get_all_gram_counts(query_literals);

    // TODO: add workload reduction options in constructor
    // if (k_reduced_queries_size_ < k_queries_size_) {
    //     workload_reduction(query_literals, pre_suf_count);
    // }

    /** referring to the implementation detail in section 2.2
        Example 3 and the paragraph above 
        Note: in our implementation, 
              query_literals, and presuf_count will be modified, 
              keeping only the reduced set of queries.**/
    auto candidates = candidate_gram_set_gen(query_literals, pre_suf_count);
    size_t candidates_size = candidates.size();
    // std::cout << "candidate set generated " << std::endl;

    // Partition Q into k disjoint sets
    // Note: now query literals and presufcount is the one after workload reduction
    auto qg_gram_set = get_all_multigrams_per_query(query_literals);
    auto dist_mtx = calculate_pairwise_dist(qg_gram_set, pre_suf_count);

    std::unordered_map<size_t, std::vector<size_t>> cmap;
    if (thread_count_ >= k_queries_size_) {
        for (size_t q_idx = 0; q_idx < k_queries_size_; q_idx++) {
            cmap[q_idx] = std::vector<size_t>({q_idx});
        }
    } else {
        cmap = k_medians(dist_mtx, dist_mtx.size(), thread_count_);
    }

    // build gr_list, qg_list, rc for each partition
    std::vector<best_index::SingleThreadedIndex::job> jobs(cmap.size());
    size_t job_idx = 0;
    std::vector<std::thread> job_building_threads;
    for (const auto & [key, q_list] : cmap) {
        job_building_threads.push_back(std::thread(
            &best_index::ParallelizableIndex::build_job_local, this,
                std::ref(jobs[job_idx++]), std::cref(candidates), 
                std::cref(query_literals), std::cref(q_list)
        ));
    }
    for (auto & th : job_building_threads) {
        th.join();
    }

    /**
     * I : index key idx; 
     *     the actual string should be stored in k_index_keys_
     *     use intermediate i to reduce hash/storage overhead
     *     of string over int
     */
    std::set<size_t> index;
    std::vector<long double> benefit_global(candidates_size);

    std::vector<std::vector<long double>> benefits_local(
            jobs.size(), std::vector<long double>(candidates_size));
    // While some (q,r) uncovered AND space available
    while (!multi_all_covered(index, jobs) && 
            (k_max_num_keys_ < 0 || index.size() < k_max_num_keys_)) {
        // std::cout << "Iteration: index size " << index.size() << std::endl;

        // for every g \in G\I, set benefit_global[g] = 0
        std::fill(benefit_global.begin(), benefit_global.end(), 0);

        std::vector<std::thread> threads;
        for (int i = 0; i < jobs.size(); i++) {
            threads.push_back(std::thread(
                &best_index::ParallelizableIndex::compute_benefit, this,
                    std::ref(benefits_local[i]), std::cref(index), 
                    std::cref(jobs[i]), jobs[i].qg_list.size()
            ));
        }
        for (auto &th : threads) {
            th.join();
        }

        for (const auto & b_local : benefits_local) {
            std::transform(benefit_global.cbegin(), benefit_global.cend(), b_local.cbegin(), 
                benefit_global.begin(), std::plus<long double>());
        }

        // if exists some g with benefit_global g > 0 then
        if (std::any_of(benefit_global.cbegin(), 
                        benefit_global.cend(), 
                        [](unsigned i) { return i > 0; })
            ) {
            long double max_util = 0;
            size_t max_idx = 0;
            // for every candidate g, calculate utility g
            for (size_t i = 0; i < candidates_size; i++) {
                /**
                 * Utility = benefit_global/cost;
                 *      use numbre of records containing g as cost
                 *      by referring to footnote 3 and example 6 and 9
                 */
                long double curr_util = benefit_global[i]/(candidates_size);
                if (curr_util > max_util) {
                    max_util = curr_util;
                    max_idx = i;
                }
            }
            // I = I union {g_max} where g_max has max utility
            index.insert(max_idx);
        } else {
            break;
        }
    }
    auto selection_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << selection_time << " s" << std::endl;
    std::ostringstream log;
    log << "BEST," << thread_count_ << "," << upper_n << ",";
    log << k_threshold_ << "," << selection_time << ",";

    start = std::chrono::high_resolution_clock::now();
    for (auto idx : index) {
        auto curr_gram = candidates[idx];
        k_index_keys_.insert(curr_gram);
        for (const auto & job : jobs) {
            if (!(k_index_[curr_gram].empty())) continue;
            k_index_[curr_gram].insert(
                k_index_[curr_gram].end(), 
                job.gr_list[idx].begin(), 
                job.gr_list[idx].end()
            );
        }
    }
    auto build_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    
    std::cout << "Index Building End in " << build_time << std::endl;

    log << build_time << "," << build_time+selection_time << ",";
    log << get_num_keys() << "," << get_bytes_used() << ",";

    write_to_file(log.str());
}