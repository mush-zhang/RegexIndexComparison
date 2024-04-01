#include <thread>
#include <future>
#include <iostream>

#include "parallelizable.hpp"
#include "../../utils/utils.hpp"

// TODO: add stop token to notify all asyncs once got a false
//       https://www.geeksforgeeks.org/cpp-20-stop_token-header/
bool best_index::ParallelizableIndex::multi_all_covered(
        const std::set<unsigned int> & index, 
        const std::vector<best_index::SingleThreadedIndex::job> & jobs,
        size_t num_queries) {
    std::vector<std::future<bool>> futures;
    for (int i = 0; i < jobs.size(); i++) {
        futures.push_back(std::async(std::launch::async, 
            &best_index::ParallelizableIndex::all_covered, this,
                std::cref(index), std::cref(jobs[i]), num_queries
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
void best_index::ParallelizableIndex::select_grams(int upper_k) {
    auto start = std::chrono::high_resolution_clock::now();

    auto query_literals = get_query_literals();
    auto pre_suf_count = get_all_gram_counts(query_literals);
    
    /** referring to the implementation detail in section 2.2
        Example 3 and the paragraph above 
        Note: in our implementation, 
              query_literals, and presuf_count will be modified, 
              keeping only the reduced set of queries.**/
    auto candidates = candidate_gram_set_gen(query_literals, pre_suf_count);
    size_t candidates_size = candidates.size();
    size_t num_queries = query_literals.size();

    // Partition Q into k disjoint sets
    // Note: now query literals and presufcount is the one after workload reduction
    auto qg_gram_set = get_all_multigrams_per_query(query_literals);
    auto dist_mtx = calculate_pairwise_dist(qg_gram_set, pre_suf_count);
    auto cmap = k_medians(dist_mtx, dist_mtx.size(), k_num_clusters_);

    // build gr_list, qg_list, rc for each partition
    std::vector<best_index::SingleThreadedIndex::job> jobs(cmap.size());
    size_t job_idx = 0;
    // for (const auto & [key, r_list] : cmap) {
    //     build_job(jobs[job_idx++], candidates, query_literals, r_list);
    // }
    std::vector<std::thread> job_building_threads;
    for (const auto & [key, r_list] : cmap) {
        job_building_threads.push_back(std::thread(
            &best_index::ParallelizableIndex::build_job_local, this,
                std::ref(jobs[job_idx++]), std::cref(candidates), 
                std::cref(query_literals), std::cref(r_list)
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
    std::set<unsigned int> index;
    std::vector<long double> benefit_global(candidates_size);

    std::vector<std::vector<long double>> benefits_local(
            jobs.size(), std::vector<long double>(candidates_size));
    // While some (q,r) uncovered AND space available
    while (!multi_all_covered(index, jobs, num_queries) && 
            (k_max_num_keys_ < 0 || index.size() < k_max_num_keys_)) {
        // for every g \in G\I, set benefit_global[g] = 0
        std::fill(benefit_global.begin(), benefit_global.end(), 0);

        std::vector<std::thread> threads;
        for (int i = 0; i < jobs.size(); i++) {
            threads.push_back(std::thread(
                &best_index::ParallelizableIndex::compute_benefit, this,
                    std::ref(benefits_local[i]), std::cref(index), 
                    std::cref(jobs[i]), num_queries
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
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams End in " << elapsed << " s" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (auto idx : index) {
        k_index_keys_.insert(candidates[idx]);
        for (const auto & job : jobs) {
            k_index_[candidates[idx]].insert(
                k_index_[candidates[idx]].end(), 
                job.gr_list[idx].begin(), 
                job.gr_list[idx].end()
            );
        }
    }
    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}