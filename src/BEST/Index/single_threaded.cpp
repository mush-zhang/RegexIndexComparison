#include "single_threaded.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <random>
#include <ranges>
#include <stdexcept>

#include "../../utils/reg_utils.hpp"
#include "../../utils/utils.hpp"

// #include "../../utils/trie.hpp"

extern "C" {
    #include "../../utils/rax/rax.h"
    #include "../../utils/rax/rc4rand.h"
};

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
        g_cadinality_sum += pre_suf_count.at(g);
    }
    return g_cadinality_sum;
}

double max_dev_dist1(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    unsigned int gn_cadinality_sum = cardinality_not_in(gn_set, pre_suf_count);

    std::set<std::string> gd_set = intersect(q1_grams, q2_grams);
    unsigned int gd_cadinality_sum = 1 + cardinality_not_in(gd_set, pre_suf_count);

    return double( ((long double)gn_cadinality_sum)/((long double)gd_cadinality_sum) );
}

double max_dev_dist2(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    unsigned int gn_cadinality_sum = cardinality_not_in(gn_set, pre_suf_count);
    unsigned int gd_cadinality_sum = 1 + cardinality_not_in(q2_grams, pre_suf_count);

    return double( ((long double)gn_cadinality_sum)/((long double)gd_cadinality_sum) );
}

double max_dev_dist3(const std::set<std::string> & q1_grams, 
                     const std::set<std::string> & q2_grams,
                     const std::map<std::string, unsigned int> & pre_suf_count) {
    std::set<std::string> gn_set = diff_union(q1_grams, q2_grams);
    return (double)cardinality_not_in(gn_set, pre_suf_count);
}

size_t argmin(const std::vector<double> & v) {
    return std::distance(std::begin(v), 
                         std::min_element(std::begin(v), std::end(v)));
}

std::unordered_map<unsigned int, std::vector<size_t>> 
best_index::SingleThreadedIndex::k_medians(
        const std::vector<std::vector<double>> & dist_mtx, 
        int num_queries, int num_clusters) {
    // 1. randomly pick k queries as centroids
    std::vector<unsigned int> centroids;
    // Note: random sample function from here: https://stackoverflow.com/a/73133364
    //       does it perform better than pre-buidling the 0-k_query_size_ array?
    centroids.resize(num_clusters);
    std::ranges::sample(std::views::iota(0, num_queries), centroids.begin(), 
                        num_clusters, std::mt19937{std::random_device{}()}); 

    // c_dists: index [idx of centroid in centroids] 
    //          value [distance from current node to the centroid]
    std::vector<double> c_dists(num_clusters);
    // closest_c_idxs: index [idx of a query] 
    //                 value [current query idx of closet centroid in centroids]
    std::vector<int> closest_c_idxs(num_queries, -1);

    
    // cmap: key [query idx centroid in centroids] 
    //       value [list of query idx]
    std::unordered_map<unsigned int, std::vector<size_t>> cmap;

    bool centroid_final = false;
    size_t max_iteration = 100000;

    size_t curr_it = 0;
    while(!centroid_final && curr_it++ < max_iteration) {
        centroid_final = true;
        // 2. assign data points to nearest centroids
        std::unordered_map<unsigned int, std::vector<size_t>>().swap(cmap);
        cmap.reserve(num_clusters);
        // q_dists: index [idx of a query] 
        //          value [distance to the current query]
        for (size_t q_idx = 0; q_idx < num_queries; q_idx++) {
            auto q_dists = dist_mtx[q_idx];
            for (size_t i = 0; i < num_clusters; i++) {
                c_dists[i] = q_dists[centroids[i]];
            }
            // curr_closest: [idx of closet centroid in centroids] 
            auto curr_closest = centroids[argmin(c_dists)];
            if (closest_c_idxs[q_idx] != curr_closest) {
                centroid_final = false;
                closest_c_idxs[q_idx] = curr_closest;
            }
            cmap[curr_closest].push_back(q_idx);
        }

        std::vector<size_t> fillables;
        // 3. compute the new median as centroids for each cluster
        for (size_t c_idx = 0; c_idx < num_clusters; c_idx++) {
            auto q_idxs = cmap[centroids[c_idx]];
            if (q_idxs.empty() || q_idxs.size() == 1) {
                fillables.push_back(c_idx);
            } else {
                std::vector<double> dist_sums(q_idxs.size(), 0);
                for (size_t i = 0; i < q_idxs.size(); i++) {
                    for (size_t j = 0; j < q_idxs.size(); j++) {
                        dist_sums[i] += dist_mtx[i][j];
                    }
                }
                auto curr_c = argmin(dist_sums);
                centroids[c_idx] = q_idxs[curr_c];
            }
        }
        //3.5 empty cluster, choose a random centroid
        std::set<unsigned int> centroids_set(centroids.begin(), centroids.end());
        for (const auto & c_idx : fillables) {
            std::uniform_int_distribution<int> uni_rand_int(0, num_queries-1);
            auto rng = std::mt19937{std::random_device{}()};
            int curr_rand = uni_rand_int(rng);
            while(centroids_set.contains(curr_rand)) {
                curr_rand = uni_rand_int(rng);
            }
            centroids[c_idx] = curr_rand;
        }
    }
    return cmap;
}

std::vector<std::set<std::string>> 
best_index::SingleThreadedIndex::get_all_multigrams_per_query(
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

std::vector<std::vector<double>> 
best_index::SingleThreadedIndex::calculate_pairwise_dist(
        const std::vector<std::set<std::string>> & qg_gram_set,
        const std::map<std::string, unsigned int> & pre_suf_count) {

    double (*max_dev_dist)(const std::set<std::string> &, 
        const std::set<std::string> &,
        const std::map<std::string, unsigned int> &){ nullptr };
    switch (dist_measure_type_) {
        case dist_type::kMaxDevDist1:
            max_dev_dist = &max_dev_dist1;
            break;
        case dist_type::kMaxDevDist2:
            max_dev_dist = &max_dev_dist2;
            break;
        case dist_type::kMaxDevDist3:
            max_dev_dist = &max_dev_dist3;
            break;
        default:
            throw std::invalid_argument( "Invalid MaxDevDist function type" );
    }

    // compute pairwise distance in 2d matrix
    std::vector<std::vector<double>> dist_mtx;
    auto query_size = qg_gram_set.size();
    dist_mtx.assign(query_size, std::vector<double>(query_size));
    for (size_t i = 0; i < query_size; i++) {
        for (size_t j = 0; j < query_size; j++) {
            if (i > j) {
                dist_mtx[i][j] = dist_mtx[j][i];
            } else {
                dist_mtx[i][j] = max_dev_dist(qg_gram_set[i], qg_gram_set[j], pre_suf_count);
            }
        }
    }
    return dist_mtx;
}

/**
 * Described in section 4: Workload Reduction
 *  Essentially reducing the number of queries for gram selection,
 *  and thus reducing the space and time complexity as they are both
 *  proportional to |Q|*|R|
 */
void best_index::SingleThreadedIndex::workload_reduction(
        std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, unsigned int> & pre_suf_count) {
    auto qg_gram_set = get_all_multigrams_per_query(query_literals);
    
    auto dist_mtx = calculate_pairwise_dist(qg_gram_set, pre_suf_count);

    // use k-median centroids as the representative query subset
    auto centroids = k_medians(dist_mtx, k_queries_size_, k_reduced_queries_size_);

    std::vector<std::vector<std::string>> new_query_literals(centroids.size());
    std::map<std::string, unsigned int> new_pre_suf_count;

    size_t i = 0;
    for (const auto & [centroid_q_idx, cluster_list] : centroids) {
        new_query_literals[i++] = query_literals[centroid_q_idx];
        for (const auto & g_in_q : qg_gram_set[centroid_q_idx]) {
            new_pre_suf_count[g_in_q] = pre_suf_count[g_in_q];
        }
    }

    query_literals.swap(new_query_literals);
    pre_suf_count.swap(new_pre_suf_count);
}

std::map<std::string, unsigned int> 
best_index::SingleThreadedIndex::get_all_gram_counts(
        const std::vector<std::vector<std::string>> & query_literals) {
    rax *gram_tree = raxNew();
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

    delete gram_tree;

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

    // 4 iterate once on the dataset and 
    //   count the number of occurrance of each multigram
    // Note:
    // intially I thought of doing: once count > threshold count, then erase the key
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

    return pre_suf_count;
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
        std::vector<std::vector<std::string>> & query_literals,
        std::map<std::string, unsigned int> & pre_suf_count) {
    
    // 4.5 Optional: if we are doing workload reduction
    //     reduce the k_queries_size_ and k_queries, 
    //     also remove any multigrams in pre_suf_count that
    //     are not in queries in the new query set
    if (k_reduced_queries_size_ < k_queries_size_) {
        workload_reduction(query_literals, pre_suf_count);
    } 

    // 4. Get the smallest prefix of the remaining grams with
    //    selectivity less than c
    // Note: checking the k-1 prefix is enough because pre_suf_count has
    //       sorted keys.
    std::vector<std::string> result;
    std::string prev_key = "";
    int threshold_count = k_threshold_ * k_dataset_size_;
    for (const auto & [key, val] : pre_suf_count) {
        if (val > threshold_count) continue;
        if (prev_key.empty() ||                           // first to insert
            prev_key != key.substr(0, prev_key.size())) { // or prev not prefix
            prev_key = key;
            result.push_back(key);
        }
    }

    return result;
}

void best_index::SingleThreadedIndex::indexed_grams_in_string(
        const std::string & l, 
        const std::vector<std::string> & candidates,
        std::vector<std::set<unsigned int>> & qg_list, 
        size_t q_idx, const std::vector<bool> & candidates_filter) {

    for (size_t i = 0; i < l.size(); i++) {
        auto curr_c = l.at(i);
        std::string curr_key = l.substr(i,1);
        auto lower_it = std::lower_bound(candidates.cbegin(), candidates.cend(), curr_key);
        for (auto & it = lower_it; it != candidates.cend() && curr_key.at(0) == curr_c; ++it) {
            // check if the current key is the same with curren substr
            curr_key = *it;
            auto curr_idx = it - candidates.cbegin();
            if (curr_key == l.substr(i, curr_key.size()) && 
                (candidates_filter.empty() || candidates_filter[curr_idx])) {
                qg_list[q_idx].insert(curr_idx);
            }
        }
    }
}

void best_index::SingleThreadedIndex::indexed_grams_in_literals(
        const std::vector<std::string> & literals, 
        const std::vector<std::string> & candidates,
        std::vector<std::set<unsigned int>> & qg_list,
        size_t idx) {

    for (const auto & l : literals) {
        indexed_grams_in_string(l, candidates, qg_list, idx);
    }
}

bool best_index::SingleThreadedIndex::index_covered(
        const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job,
        size_t r_j, size_t q_k) {
    for (auto g_idx : index) {
        // the pair (q_k, r_j) is covered by current g iff
        // r_j not in G-R-list[g] AND g in Q-G-list[q_k]
        if (!sorted_list_contains(job.gr_list[g_idx], r_j) &&
            job.qg_list[q_k].find(g_idx) != job.qg_list[q_k].cend()) {
        
            return true;
        }
    }
    return false;
}

// TODO: to speed up, it is possible to store all covered pairs
bool best_index::SingleThreadedIndex::all_covered(
        const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job, 
        size_t query_size) {
    for (size_t k = 0; k < query_size; k++) {
        for (auto j : job.rc) {
            if (!index_covered(index, job, j, k)) {
                return false;
            }
        }
    }
    return true;
}

void best_index::SingleThreadedIndex::compute_benefit(
        std::vector<long double> & benefit, 
        const std::set<unsigned int> & index, 
        const best_index::SingleThreadedIndex::job & job, 
        size_t num_queries) {
    
    std::fill(benefit.begin(), benefit.end(), 0);

    // for every query q \in Q
    for (size_t k = 0; k < num_queries; k++) {
        // for every record r in R_c
        for (auto j : job.rc) {
            // for each gram g \in Q-G-list[q_k]\I
            for (auto g_idx : job.qg_list[k]) {
                // ... and if 1. g not in r_j 
                //        AND 2. (q_k, r_j) not covered by any g \in I
                if (index.find(g_idx) == index.cend() &&
                    !sorted_list_contains(job.gr_list[g_idx], j) &&
                    (!index_covered(index, job, j, k)) ) {
                    
                    benefit[g_idx]++;
                }
            }
        }
    }
}

std::vector<std::vector<std::string>> 
best_index::SingleThreadedIndex::get_query_literals() {
    std::vector<std::vector<std::string>> query_literals;
    for (const auto & q : k_queries_) {
        std::vector<std::string> literals = extract_literals(q);
        query_literals.push_back(literals);
    }
    return query_literals;
}

void best_index::SingleThreadedIndex::build_qg_list(
        std::vector<std::set<unsigned int>> & qg_list,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals) {
    auto num_queries = query_literals.size();
    qg_list.assign(num_queries, std::set<unsigned int>());
    for (size_t q_idx = 0; q_idx < num_queries; q_idx++) {
        const auto & literals = query_literals[q_idx];
        indexed_grams_in_literals(literals, candidates, qg_list, q_idx);
    }
}

void build_gr_list_rc_helper(best_index::SingleThreadedIndex::job & job, 
        size_t set_idx, size_t r_idx, 
        const std::vector<std::set<unsigned int>> & rg_list) {
    const std::set<unsigned int> & set_of_exists_grams = rg_list[set_idx];
    if (!set_of_exists_grams.empty()) {
        job.rc.push_back(r_idx);
    }
    for (unsigned int g_idx : set_of_exists_grams) {
        job.gr_list[g_idx].push_back(r_idx);
    }
}

void best_index::SingleThreadedIndex::build_gr_list_rc(
        best_index::SingleThreadedIndex::job & job, 
        size_t candidates_size,  
        unsigned int dataset_size,
        const std::vector<std::set<unsigned int>> & rg_list) {
    job.gr_list.assign(candidates_size, std::vector<unsigned int>());
    for (size_t r_idx = 0; r_idx < dataset_size; r_idx++) {
        build_gr_list_rc_helper(job, r_idx, r_idx, rg_list);
    }
}

void best_index::SingleThreadedIndex::build_job(
        best_index::SingleThreadedIndex::job & job,
        const std::vector<std::string> & candidates, 
        const std::vector<std::vector<std::string>> & query_literals) {

    build_qg_list(job.qg_list, candidates, query_literals);
    std::vector<std::set<unsigned int>> rg_list(k_dataset_size_);
    for (size_t i = 0; i < k_dataset_size_; i++) {
        indexed_grams_in_string(k_dataset_[i], candidates, rg_list, i);
    }
    build_gr_list_rc(job, candidates.size(), k_dataset_size_, rg_list);
}

// Algorithm 3 in Figure 4
// Improved greedy gram selection algorightm
//   TODO: no point of seperating select gram and build index;
//         only do that if we need some consistent interface later for experiments
void best_index::SingleThreadedIndex::select_grams(int upper_k) {
    auto start = std::chrono::high_resolution_clock::now();
    auto query_literals = get_query_literals();
    auto pre_suf_count = get_all_gram_counts(query_literals);
    
    /** referring to the implementation detail in section 2.2
        Example 3 and the paragraph above **/
    auto candidates = candidate_gram_set_gen(query_literals, pre_suf_count);
    size_t candidates_size = candidates.size();
    size_t num_queries = query_literals.size();

    best_index::SingleThreadedIndex::job job;
    build_job(job, candidates, query_literals);

    /**
     * I : index key idx; 
     *     the actual string should be stored in k_index_keys_
     *     use intermediate i to reduce hash/storage overhead
     *     of string over int
     */
    std::set<unsigned int> index;
    std::vector<long double> benefit(candidates_size);
    // While some (q,r) uncovered AND space available
    while (!all_covered(index, job, num_queries) && 
            (k_max_num_keys_ < 0 || index.size() < k_max_num_keys_)) {

        compute_benefit(benefit, index, job, num_queries);

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
                long double curr_util = benefit[i]/(candidates_size);
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
    std::map<std::string, size_t> gram_to_candidate_idx_map;
    for (auto idx : index) {
        k_index_keys_.insert(candidates[idx]);
        gram_to_candidate_idx_map.insert({candidates[idx], idx});
    }
    size_t pos_list_idx = 1;
    for (auto it = k_index_keys_.begin(); it != k_index_keys_.end(); ++it) {
        auto key = iter_to_key(it);

        auto idx = gram_to_candidate_idx_map.at(*it);
        k_idx_lists_.push_back(job.gr_list[idx]);

        k_index_.btree_insert(key, reinterpret_cast<char*>(pos_list_idx++));
    }

    elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Index Building End in " << elapsed << std::endl;
}

// Algorithm 2 in Figure 3
void best_index::SingleThreadedIndex::build_index(int upper_k) {
    select_grams();
}

