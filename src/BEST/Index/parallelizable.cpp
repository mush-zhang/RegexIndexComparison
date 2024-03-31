#include <thread>
#include <iostream>

#include "parallelizable.hpp"
#include "../../utils/utils.hpp"

std::vector<std::set<std::string>> 
best_index::ParallelizableIndex::get_all_multigrams_per_query(
        const std::vector<std::vector<std::string>> & query_literals) {
    auto query_size = query_literals.size();
    std::vector<std::set<std::string>> qg_gram_set(query_size);
    for (size_t q_idx = 0; q_idx < query_size; q_idx++) {
        const auto & literals = query_literals[q_idx];
        for (const auto & l : literals) {
            for (size_t i = 0; i < l.size(); i++) {
                for (size_t j = i; j < l.size(); j++) {
                    qg_gram_set[q_idx].insert(l.substr(i, j-i+1));
                }
            }
        }
    }
    return qg_gram_set;
}

void run_job(std::vector<long double> & benefit, const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job, size_t num_queries) {
    
    std::fill(benefit.begin(), benefit.end(), 0);

    // for every query q \in Q
    for (size_t k = 0; k < num_queries; k++) {
        // for every record r in R_c
        for (auto j : job.rc) {
            // for each gram g \in Q-G-list[q_k]\I
            for (auto g_idx : job.qg_list[k]) {
                // ... and if 1. g not in r_j 
                //        AND 2. (q_k, r_j) not covered by any g \in I
                if (index.find(g_idx) == index.end() &&
                    !sorted_list_contains(job.gr_list[g_idx], j) &&
                    (!index_covered(index, job.gr_list, job.qg_list, j, k)) ) {
                    
                    benefit[g_idx]++;
                }
            }
        }
    }
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
    for (const auto & [key, r_list] : cmap) {
        auto job = jobs[job_idx++];
        build_qg_list(job.qg_list, num_queries, candidates, query_literals);
        std::vector<std::set<unsigned int>> rg_list(r_list.size());
        for (auto & i : r_list) {
            indexed_grams_in_string(k_dataset_[i], candidates, rg_list, i);
        }
        build_gr_list_rc(job.gr_list, job.rc, candidates_size, r_list, rg_list);
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
    // TODO: all_cover for each job
    while (!all_covered(rc, index, gr_list, qg_list, num_queries) && 
            (k_max_num_keys_ < 0 || index.size() < k_max_num_keys_)) {
        // for every g \in G\I, set benefit_global[g] = 0
        std::fill(benefit_global.begin(), benefit_global.end(), 0);

        std::vector<std::thread> threads;
        for (int i = 0; i < jobs.size(); i++) {
            threads.push_back(std::thread(
                run_job, benefits_local[i], index, jobs[i], num_queries
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
        k_index_[candidates[idx]] = gr_list[idx];
    }
    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}