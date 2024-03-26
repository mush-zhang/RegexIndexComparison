#include "single_threaded.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <map>
#include <random>
#include <algorithm>
#include "../../utils/reg_utils.hpp"

// #include "../../utils/trie.hpp"

extern "C" {
    #include "../../utils/rax/rax.h"
    #include "../../utils/rax/rc4rand.h"
};

template<class T, class U>
bool sorted_list_contains(const std::vector<T>& container, const U& v)
{
    auto it = std::lower_bound(
        container.begin(),
        container.end(),
        v,
        [](const T& l, const U& r){ return l < r; });
    return it != container.end() && *it == v;
}

// TODO: measure the performance of rax and trie_type
int insert_gram_to_tree(rax * const gram_tree, 
                                                const std::string & l) {
    char cl[l.size()+1];
    strcpy(cl, l.c_str());
    unsigned char* ucl = reinterpret_cast<unsigned char*>(cl);
    return raxTryInsert(gram_tree, &ucl[0], l.size(), NULL, NULL);
}

std::vector<std::string> generate_path_labels(rax * const gram_tree) {
    std::vector<std::string> path_labels;
    raxIterator iter;
    raxStart(&iter, gram_tree);
    raxSeek(&iter,"^",NULL,0);

    while(raxNext(&iter)) {
        std::string curr((char*)iter.key, (int)iter.key_len);
        path_labels.push_back(curr);
    }
    raxStop(&iter);
    return path_labels;
}

std::set<std::string> diff_union(const std::set<std::string> & q1_grams, 
                                   const std::set<std::string> & q2_grams) {
    // Calcuate G_n = (G_1 - G_2) \union (G_2 - G_1)
    std::set<std::string> result;
    std::set_difference(q1_grams.begin(), q1_grams.end(), 
                        q2_grams.begin(), q2_grams.end(),
                        std::inserter(result, result.end()));
    std::set_difference(q2_grams.begin(), q2_grams.end(), 
                        q1_grams.begin(), q1_grams.end(),
                        std::inserter(result, result.end())); 
    return result;   
}   

std::set<std::string> intersect(const std::set<std::string> & q1_grams, 
                                   const std::set<std::string> & q2_grams) {
    // Calcuate G_d = G_1 \intersect G_2
    std::set<std::string> result;
    std::set_intersection(q1_grams.begin(), q1_grams.end(), 
                          q2_grams.begin(), q2_grams.end(),
                          std::inserter(result, result.end()));
    return result;   
} 

unsigned int cardinality_not_in(const std::set<std::string> & g_set, 
                                const std::map<std::string, unsigned int> & pre_suf_count) {
    unsigned int g_cadinality_sum = 0;
    for (const auto & g : g_set) {
        g_cadinality_sum += pre_suf_count[g];
    }
    return g_cadinality_sum;
}

double max_dev_dist1(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    unsigned int gn_cadinality_sum = cardinality_not_in(gn_set, pre_suf_count);

    std::set<std::string> gd_set = intersect(q1_grams, q2_grams);
    unsigned int gd_cadinality_sum = cardinality_not_in(gd_set, pre_suf_count);

    return double( ((long double)gn_cadinality_sum)/((long double)gd_cadinality_sum) );
}

double max_dev_dist2(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    unsigned int gn_cadinality_sum = cardinality_not_in(gn_set, pre_suf_count);
    unsigned int gd_cadinality_sum = cardinality_not_in(q2, pre_suf_count);

    return double( ((long double)gn_cadinality_sum)/((long double)gd_cadinality_sum) );
}

double max_dev_dist3(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    return (double)cardinality_not_in(gn_set, pre_suf_count);
}

void k_medians(const std::vector<std::vector<double>> & dist_mtx, 
        int query_size_old, int query_size_new) {
    // 1. randomly pick k queries as centriods
    std::vector<unsigned int> centriods;
    // Note: random sample function from here: https://stackoverflow.com/a/73133364
    //       does it perform better than pre-buidling the 0-k_query_size_ array?
    centriods.resize(query_size_new);
    std::ranges::sample(std::views::iota(0, query_size_old), centriods.begin(), 
                        query_size_new, std::mt19937{std::random_device{}()}); 

    bool centriod_final = false;
    while(centriod_final) {
        centriod_not_final = 
    }
    // 2. assign data points to nearest centriods

    // compute the new median as centriods for each cluster

    // 
}

/**
 * Described in section 4: Workload Reduction
 *  Essentially reducing the number of queries for gram selection,
 *  and thus reducing the space and time complexity as they are both
 *  proportional to |Q|*|R|
 */
void best_index::SingleThreadedIndex::workload_reduction(
        const std::vector<std::vector<std::string>> & query_literals,
        const std::map<std::string, unsigned int> & pre_suf_count) {
    std::vector<std::set<std::string>> qg_gram_set(k_queries_size_);
    for (size_t q_idx = 0; q_idx < k_queries_size_; q_idx++) {
        const auto & literals = query_literals[q_idx];
        for (const auto & l : literals) {
            for (size_t i = 0; i < l.size(); i++) {
                for (size_t j = i; j < l.size(); j++) {
                    qg_gram_set[q_idx].insert(l.substr(i, j-i+1))
                }
            }
        }
    }

    // compute pairwise distance in 2d matrix
    std::vector<std::vector<double>> dist_mtx;
    dist_mtx.assign(k_queries_size_, std::vector<double>(k_queries_size_));
    for (size_t i = 0; i < k_queries_size_; i++) {
        for (size_t j = 0; j < k_queries_size_; j++) {
            if (i > j) {
                dist_mtx[i][j] = dist_mtx[j][i];
            } else {
                dist_mtx[i][j] = max_dev_dist1(qg_gram_set[i], qg_gram_set[j], pre_suf_count);
            }
        }
    }
}

/**Described in section 5: Candidate Set Generation
 * Note: it generate the set of prefixes of the set of all suffixes of the string set
 *       which isn't essentially the set of all substrings/multigrams.
 *       is it due to storage consideration?
 *       Anyway, we are storing the set of all multigrams in a map and record their count
 *       so that we tranverse the dataset only once
 *       Or else if we generate the prefix set for each single suffix on the fly
 *       it will take several scans on the dataset; will only be fine if dataset is small
 *       I feel it is similar to FREE in generating prefix free kinda set with threshold limit
 **/
std::vector<std::string> best_index::SingleThreadedIndex::candidate_gram_set_gen(
        std::vector<std::vector<std::string>> & query_literals) {
   
    rax *gram_tree = raxNew();
    std::vector<std::string> result;
    // 1. Build suffix tree using all queries
    for (const auto & literals : query_literals) {
        for (const auto & l : literals) {
            for (size_t i = 0; i < l.size(); i++) {
                insert_gram_to_tree(gram_tree, l.substr(i, l.size() - i));
            }
        }
    }
    // 2. get all path labels in sorted list I guess
    auto path_labels = generate_path_labels(gram_tree);

    // 3. get all prefixes of path labels; 
    //    store them in an ordered map
    std::map<std::string, unsigned int> pre_suf_count;
    std::string prev = "";
    for (const auto & pl : path_labels) {
        for (size_t i = 0; i < pl.size(); i++) {
            // get all prefixes
            pre_suf_count.insert({pl.substr(0, i+1), 0});
        }
    }

    // 3.5 iterate once on the dataset and 
    //     count the number of occurrance of each multigram
    // Note:
    // intially I thought of doing: once count > threshold count, then erase the key
    int threshold_count = k_threshold_ * k_dataset_size_;
    for (const auto & line : k_dataset_) {
        for (size_t i = 0; i < line.size(); i++) {
            auto curr_c = line.at(i);
            std::string curr_key = line.substr(i,1);
            auto lower_it = pre_suf_count.lower_bound(curr_key);

            for (auto & it = lower_it; it != pre_suf_count.end() && curr_key.at(0) == curr_c; ++it) {
                // check if the current key is the same with curren substr
                curr_key = it->first;
                if (curr_key == line.substr(i, curr_key.size())) {
                    pre_suf_count[curr_key]++;
                }
            }
        }
    }

    // 

    // 4. Get the smallest prefix of the remaining grams with
    //    selectivity less than c
    std::string prev_key = "";
    for (const auto & [key, val] : pre_suf_count) {
        if (val > threshold_count) continue;
        if (prev_key.empty() ||                           // first to insert
            prev_key != key.substr(0, prev_key.size())) { // or prev not prefix
            prev_key = key;
            result.push_back(key);
        }
    }
    delete gram_tree;

    return result;
}

void grams_in_string(const std::string & l, 
                     const std::vector<std::string> & candidates,
                     std::vector<std::set<unsigned int>> & g_list, 
                     size_t idx) {
    for (size_t i = 0; i < l.size(); i++) {
        auto curr_c = l.at(i);
        std::string curr_key = l.substr(i,1);
        auto lower_it = std::lower_bound(candidates.cbegin(), candidates.cend(), curr_key);

        for (auto & it = lower_it; it != candidates.cend() && curr_key.at(0) == curr_c; ++it) {
            // check if the current key is the same with curren substr
            curr_key = *it;
            if (curr_key == l.substr(i, curr_key.size())) {
                g_list[idx].insert(it - candidates.cbegin());
            }
        }
    }
}

void grams_in_literals(const std::vector<std::string> & literals, 
                       const std::vector<std::string> & candidates,
                       std::vector<std::set<unsigned int>> & g_list,
                       size_t idx) {

    for (const auto & l : literals) {
        grams_in_string(l, candidates, g_list, idx);
    }
}

bool index_covered(const std::set<unsigned int> & index, 
                   const std::vector<std::vector<unsigned int>> & gr_list,
                   const std::vector<std::set<unsigned int>> & qg_list,
                   size_t r_j, size_t q_k) {
    for (auto g_idx : index) {
        // the pair (q_k, r_j) is covered by current g iff
        // r_j not in G-R-list[g] AND g in Q-G-list[q_k]
        if (!sorted_list_contains(gr_list[g_idx], r_j) &&
            qg_list[q_k].find(g_idx) != qg_list[q_k].end()) {
            
            return true;
        }
    }
    return false;
}

// TODO: to speed up, it is possible to store all covered pairs
bool all_covered(const std::vector<unsigned int> & rc, const std::set<unsigned int> & index, 
        const std::vector<std::vector<unsigned int>> & gr_list,
        const std::vector<std::set<unsigned int>> & qg_list, size_t query_size){
    for (size_t k = 0; k < query_size; k++) {
        for (auto j : rc) {
            if (!index_covered(index, gr_list, qg_list, j, k)) {
                return false;
            }
        }
    }
    return true;
}

// Algorithm 3 in Figure 4
// Improved greedy gram selection algorightm
//   TODO: no point of seperating select gram and build index;
//         only do that if we need some consistent interface later for experiments
void best_index::SingleThreadedIndex::select_grams(int upper_k) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<std::string>> query_literals;
    for (const auto & q : k_queries_) {
        std::vector<std::string> literals = extract_literals(q);
        query_literals.push_back(literals);
    }
    /** referring to the implementation detail in section 2.2
        Example 3 and the paragraph above **/
    std::vector<std::string> candidates = candidate_gram_set_gen(query_literals);
    size_t candidates_size = candidates.size();
    /**
     * Q-G-list: vector in order of q, where each element is
     *      set of idx of g \in candidates s.t. g \in q
     */
    std::vector<std::set<unsigned int>> qg_list(k_queries_size_);
    for (size_t i = 0; i < k_queries_size_; i++) {
        const auto & literals = query_literals[i];
        grams_in_literals(literals, candidates, qg_list, i);
    }
    /**
     * G-R-list: vector in order of g, where each element is
     *      set of idx of r \in candidates s.t. g \in r
     *   Note: to avoid multiple scans of the dataset, 
     *        build R-G-list first
     * R_c: set of idx of r s.t. \exists some g \in r
     */
    std::vector<std::set<unsigned int>> rg_list(k_dataset_size_);
    for (size_t i = 0; i < k_dataset_size_; i++) {
        grams_in_string(k_dataset_[i], candidates, rg_list, i);
    }
    // Using sorted vector for gr_list elements and rc.
    std::vector<std::vector<unsigned int>> gr_list(candidates_size);
    std::vector<unsigned int> rc;
    for (size_t i = 0; i < k_dataset_size_; i++) {
        const std::set<unsigned int> & set_of_exists_grams = rg_list[i];
        if (!set_of_exists_grams.empty()) {
            rc.push_back(i);
        }
        for (unsigned int g_idx : set_of_exists_grams) {
            gr_list[g_idx].push_back(i);
        }
    }
    /**
     * I : index key idx; 
     *     the actual string should be stored in k_index_keys_
     *     use intermediate i to reduce hash/storage overhead
     *     of string over int
     */
    std::set<unsigned int> index;
    std::vector<long double> benefit(candidates_size);
    // While some (q,r) uncovered AND space available
    while (!all_covered(rc, index, gr_list, qg_list, k_queries_size_) && 
            (k_max_num_keys_ < 0 || index.size() < k_max_num_keys_)) {
        // for every g \in G\I, set benefit[g] = 0
        std::fill(benefit.begin(), benefit.end(), 0);
        // for every query q \in Q
        for (size_t k = 0; k < k_queries_size_; k++) {
            // for every record r in R_c
            for (auto j : rc) {
                // for each gram g \in Q-G-list[q_k]\I
                //  TODO: would std::set_difference perform better
                for (auto g_idx : qg_list[k]) {
                    // ... and if 1. g not in r_j 
                    //        AND 2. (q_k, r_j) not covered by any g \in I
                    if (index.find(g_idx) == index.end() &&
                        !sorted_list_contains(gr_list[g_idx], j) &&
                        (!index_covered(index, gr_list, qg_list, j, k)) ) {
                        
                        benefit[g_idx]++;
                    }
                }
            }
        }
        // if exists some g with benefit g > 0 then
        if (std::any_of(benefit.cbegin(), benefit.cend(), [](unsigned i) { return i > 0; })) {
            long double max_util = 0;
            size_t max_idx = 0;
            // for every candidate g, calculate utility g
            for (size_t i = 0; i < candidates_size; i++) {
                /**
                 * Utility = benefit/cost;
                 *      use numbre of records containing g as cost
                 *      by referring to footnote 3 and example 6 and 9
                 */
                long double curr_util = benefit[i]/(gr_list[i].size());
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
        k_index_.insert({candidates[idx], gr_list[idx]});
    }
    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}

// Algorithm 2 in Figure 3
void best_index::SingleThreadedIndex::build_index(int upper_k) {
    select_grams();
}

