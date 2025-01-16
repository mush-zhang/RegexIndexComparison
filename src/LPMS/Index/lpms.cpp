#include <exception>
#include <cassert>

#include "lpms.hpp"
#include "../../utils/utils.hpp"

#ifdef NDEBUG
#define assert(x) (void(0))
#endif

// static const double k_deterministic_threshold_ = 0.99;

using gram_set = std::unordered_set<std::string>;

/**
 * @brief Check if the current ngram is already visited in the line;
 *        if not, increment its count of #occurance in the dataset/queries
 *        and also record the new ngram
 * @param kgrams_count map from gram_idx to #occurance in dataset/queries
 * @param key ngram as the map key
 * @param visited_kgrams set of all ngrams that is already visited in the current line
 * @param kgrams hash map of all ngrams and their artificial index
 * @return bool If the current key is a new ngram
 */
template <typename T, class hash_T>
static void insert_or_increment(std::unordered_map<size_t, long double> & kgrams_count, 
                                T & key,
                                std::unordered_set<T, hash_T> & visited_kgrams,
                                std::unordered_map<T, size_t, hash_T> & kgrams) {
    size_t curr_idx = kgrams.size();
    if (kgrams.contains(key)) {
        curr_idx = kgrams.at(key);
        kgrams_count[curr_idx]++;
    } else {
        kgrams.insert({ key, curr_idx });
        kgrams_count.insert({ curr_idx, 1 });
    }
    visited_kgrams.insert(key);
}

void lpms_index::LpmsIndex::get_unigram_r(
        std::unordered_map<size_t, long double> & uni_count,
        std::unordered_map<char, size_t> & unigrams,
        std::vector<std::vector<size_t>> & uni_gr_map) {
    for (size_t r = 0; r < k_dataset_size_; r++) {
        auto line = k_dataset_[r];
        std::unordered_set<char> visited_unigrams;
        for (size_t i = 0; i < line.size(); i++) {
            char c = line.at(i);
            if (visited_unigrams.find(c) == visited_unigrams.end()) {
                insert_or_increment(uni_count, c, visited_unigrams, unigrams);
                if (uni_count.size() > uni_gr_map.size()) {
                    uni_gr_map.push_back(std::vector<size_t>{r});
                } else {
                    uni_gr_map[unigrams.at(c)].push_back(r);
                }
            }
        }
    }
}

void get_unigram_q(const std::vector<std::vector<std::string>> & q_lits, 
                   std::unordered_map<size_t, long double> & uni_count,
                   std::unordered_map<char, size_t> & unigrams, 
                   std::vector<std::set<size_t>> & uni_qg_map) {

    for (size_t q = 0; q < q_lits.size(); ++q) {
        std::unordered_set<char> visited_unigrams;
        for (const auto & lit : q_lits[q]) {
            for (size_t i = 0; i < lit.size(); i++) {
                char c = lit.at(i);
                if (visited_unigrams.find(c) == visited_unigrams.end()) {
                    insert_or_increment(uni_count, c, visited_unigrams, unigrams);
                    assert(uni_qg_map.size() > q && "line 75 uni_qg_map size < q");
                    uni_qg_map[q].insert(unigrams.at(c));
                }             
            }
        }
    }
}

void lpms_index::LpmsIndex::get_kgrams_r(
        std::unordered_map<size_t, long double> & r_count,
        std::unordered_map<std::string, size_t> & kgrams,
        std::vector<std::vector<size_t>> & gr_map,
        const std::unordered_set<std::string> & expand, size_t k) {
    // get all grams whose prefix in expand
    for (size_t r = 0; r < k_dataset_size_; r++) {
        auto line = k_dataset_[r];
        gram_set visited_kgrams;
        for (size_t i = 0; i+k <= line.size(); i++) {
            auto curr_kgram = line.substr(i, k);
            // not seen in current line and in expand
            if (visited_kgrams.find(curr_kgram) == visited_kgrams.end() &&
                    expand.find(line.substr(i, k-1)) != expand.end()) {
                insert_or_increment(r_count, curr_kgram, visited_kgrams, kgrams);
                if (r_count.size() > gr_map.size()) {
                    gr_map.push_back(std::vector<size_t>{r});
                } else {
                    assert(gr_map.size() > kgrams.at(curr_kgram) && "line 103 gr_map size");
                    gr_map[kgrams.at(curr_kgram)].push_back(r);
                }
            }
        }
    }
} 

void get_kgrams_q(const std::vector<std::vector<std::string>> & q_lits,
        std::unordered_map<size_t, long double> & q_count,
        std::unordered_map<std::string, size_t> & kgrams,
        std::vector<std::set<size_t>> & qg_map,
        const std::unordered_set<std::string> & expand, size_t k) {

    for (size_t q = 0; q < q_lits.size(); ++q) {
        gram_set visited_kgrams;
        for (const auto & lit : q_lits[q]) {
            for (size_t i = 0; i+k < lit.size(); i++) {
                auto curr_kgram = lit.substr(i, k);
                // not seen in current lit and in expand
                if (visited_kgrams.find(curr_kgram) == visited_kgrams.end() &&
                        expand.find(lit.substr(i, k-1)) != expand.end()) {
                    insert_or_increment(q_count, curr_kgram, visited_kgrams, kgrams);
                    assert(qg_map.size() > kgrams.at(curr_kgram) && "line 124 gr_map size");
                    qg_map[q].insert(kgrams.at(curr_kgram));
                }            
            }
        }
    }
} 

/**
 * @brief Apply LPMS-D or LPMS-R to find the vector x
 * 
 * @param r_count Key is gram_idx, value is the #records with this gram
 * @param q_count Key is gram_idx, value is the #queries with this gram
 * @param qg_map Key is query_idx, value is the set of all gram_idxs in the query
 * @return std::vector<bool> vector of bool for if the gram is kept
 */
std::vector<bool> lpms_index::LpmsIndex::build_model(size_t k,
        const std::unordered_map<size_t, long double> & r_count, 
        const std::unordered_map<size_t, long double> & q_count, 
        const std::vector<std::set<size_t>> & qg_map,
        GRBEnv* env) {
    GRBVar* x = 0;

    size_t num_grams = r_count.size();
    size_t num_queries = qg_map.size();
    std::vector<bool> x_result(num_grams, false);

    try {
        // Model
        GRBModel model = GRBModel(*env);
        model.set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_StringAttr_ModelName, "model");
        model.set(GRB_IntParam_Threads, thread_count_);

        // minimize \sum_{g \in G} c_g x_g ; x \in {0, 1}
        // Gurobi by default minimize the objective

        // 5. populate vector c where c_g is 
        //    (r_count of g) / (|g| * q_count of g) 
        //    c length num_gram
        // Note: Definition of c in Section 3.1, formulae (5)
        x = model.addVars(num_grams, GRB_CONTINUOUS);
        double smax = 0;
        double smin = k_dataset_size_;
        for (const auto & [g_idx, curr_r_count] : r_count) {
            long double curr_q_count = 0;
            if (q_count.contains(g_idx)) {
                curr_q_count = q_count.at(g_idx); 
            }
            double c_g = (curr_r_count / k) * curr_q_count;
            x[g_idx].set(GRB_DoubleAttr_Obj, c_g);
            x[g_idx].set(GRB_StringAttr_VarName, "x_" + std::to_string(g_idx));
            if (curr_r_count > smax) smax = curr_r_count;
            if (curr_r_count < smin) smin = curr_r_count;
        }

        double mstar = 0;
        // qg_map : key: q; value: set of g in q
        for (size_t q_idx = 0; q_idx < num_queries; ++q_idx) {
            auto curr_grams_in_q = qg_map[q_idx];
            size_t b_q = k_dataset_size_;
            if (curr_grams_in_q.empty()) {
                b_q = 0;
            } else {
                if (curr_grams_in_q.size() > mstar) mstar = curr_grams_in_q.size();
                // 4. populate vector b, where b_q denote the smallest
                //    r_count among all grams in query q; b length num_query
                // Note: Definition of b in Section 3.1, formulae (3)
                for (const auto & curr_gram_idx : curr_grams_in_q) {
                    long double curr_r_count = 0;
                    if (r_count.contains(curr_gram_idx)) {
                        curr_r_count = q_count.at(curr_gram_idx);
                    }
                    if (curr_r_count < b_q) {
                        b_q = curr_r_count;
                    }
                }
            }
            // 3. populate matrix A, where A_{q, g} denote 
            //    r_count of g if g \in q; A has size num_query * num_gram
            // Note: Definition of A in Section 3.1, formulae (2)
            // Note: so whenever some gram g is in some query q, 
            //    the value of the matrix entry is the count of 
            //    number of records in the whole dataset that 
            //    contains this gram
            GRBLinExpr Ax_q = 0;

            // This should follow the order of the temp vector grams_ordered!
            for (size_t g_inner_idx = 0; g_inner_idx < num_grams; ++g_inner_idx) {
                size_t A_qg = 0;

                if (r_count.contains(g_inner_idx) && qg_map[q_idx].contains(g_inner_idx)) {
                    A_qg = r_count.at(g_inner_idx);
                }
                Ax_q += A_qg*x[g_inner_idx];
            }
            
            model.addConstr(Ax_q >= b_q, "Ax_" + std::to_string(q_idx));
        }

        // Solve
        model.optimize();

        // 6. use lp solver with deterministic relaxation 
        //    or random rounding to find x
        switch (k_relaxation_type_) {
            case kDeterministic: {
                double threshold = (smax*mstar > 0) ? smin/(smax*mstar) : 1;
                for (size_t g_idx = 0; g_idx < num_grams; ++g_idx) {
                    if (x[g_idx].get(GRB_DoubleAttr_X) > threshold) {
                        x_result[g_idx] = true;
                    }
                }
                break;
            }
            case kRandomized:
                for (size_t g_idx = 0; g_idx < num_grams; ++g_idx) {
                    x_result[g_idx] = (std::rand() / double(RAND_MAX)) < (x[g_idx].get(GRB_DoubleAttr_X));
                }
                break;
        }
    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (std::exception e2) {
        std::cout << "Exception during optimization" << std::endl;
        std::cout << e2.what() << std::endl;
    }
    delete x;
    return x_result;
}

void lpms_index::LpmsIndex::uni_special(std::unordered_set<std::string> & expand, 
        const std::vector<std::vector<std::string>> & query_literals, GRBEnv * env) {
    std::unordered_map<size_t, long double> unigrams_r_count;
    std::unordered_map<char, size_t> unigrams; 
    std::vector<std::vector<size_t>> uni_gr_map;

    get_unigram_r(unigrams_r_count, unigrams, uni_gr_map);

    std::vector<std::set<size_t>> uni_qg_map(k_queries_size_, std::set<size_t>());
    std::unordered_map<size_t, long double> unigrams_q_count;
    get_unigram_q(query_literals, unigrams_q_count, unigrams, uni_qg_map);

    // 3. build matrix A for unigram
    std::vector<bool> mask_uni = build_model(1, unigrams_r_count, unigrams_q_count, uni_qg_map, env);

    for (const auto & [c, idx] : unigrams) {
        std::string curr_kgram = std::string(1, c);
        if (mask_uni[idx]) {
            // 7. Move all multigtams in the children set
            //    whose associated value in x is 1 to G (the index)
            k_index_keys_.insert(curr_kgram); 
            k_index_.insert({ curr_kgram, uni_gr_map.at(idx) });
            if (k_index_keys_.size() >= key_upper_bound_) break;
        } else {
            // 8. Those multigrams remainig become the new expand set
            expand.insert(curr_kgram);
        }
    }

}

// Algorithm 1: LPMS multigram selection algorithm
void lpms_index::LpmsIndex::select_grams(int upper_n) {
    GRBEnv* env = new GRBEnv();
    // env->set("OutputFlag", 0);

    // Initialize a empty expand set
    gram_set expand; // stores useless prefix
    auto query_literals = get_query_literals();

    uni_special(expand, query_literals, env);
    size_t k = 2;

    // 9. stop until expand set empty.
    while (!expand.empty()) {

        // 1. for each multigram in expand set, 
        //    append a member in alphabet and save to children set

        // 2. remove all multigrams g in children set that
        //    for all k, g \notin CandidateSet(k, q)
        // Note: I guess like the gram g is not in any query

        // Important Note: This seems only effective if it is dealing with bio dataset
        //     which has small sized alphabet. The removal would require scanning the
        //     whole dataset after all, we might well just insert all occurred suffixes

        std::unordered_map<size_t, long double> r_count;
        std::unordered_map<std::string, size_t> kgrams; 
        std::vector<std::vector<size_t>> gr_map;

        get_kgrams_r(r_count, kgrams, gr_map, expand, k);
        std::vector<std::set<size_t>> qg_map(k_queries_size_, std::set<size_t>());
        std::unordered_map<size_t, long double> q_count;
        get_kgrams_q(query_literals, q_count, kgrams, qg_map, expand, k);

        if (q_count.empty()) break;
        
        std::vector<bool> mask = build_model(k, r_count, q_count, qg_map, env);
        
        decltype(expand)().swap(expand);

        for (const auto & [curr_kgram, idx] : kgrams) {
            if (mask[idx]) {
                // 7. Move all multigtams in the children set
                //    whose associated value in x is 1 to G (the index)
                k_index_keys_.insert(curr_kgram); 
                assert(gr_map.size() > idx && "line 324 gr_map size < idx");
                k_index_[curr_kgram] = gr_map.at(idx);
                if (k_index_keys_.size() >= key_upper_bound_) goto END_SELECT;
            } else {
                // 8. Those multigrams remainig become the new expand set
                expand.insert(curr_kgram);
            }
        }

        k++;
    }
    END_SELECT:;
    delete env;
}

void lpms_index::LpmsIndex::build_index(int upper_n) {
    auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_n);
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "Select Grams and Index Building End in " << elapsed << " s" << std::endl;
    std::ostringstream log;    
    std::string method_suffix;
    if (k_relaxation_type_ == kDeterministic) {
        method_suffix = "DETERM";
    } else if (k_relaxation_type_ == kRandomized) {
        method_suffix = "RANDOM";
    } 
    log << "LPMS-" << method_suffix << "," << thread_count_ << "," << upper_n << ",";
    log << "-1,"  << key_upper_bound_ << "," << k_queries_size_ << ",";
    log << elapsed << ",";  // selectivity threshold, select time
    log << "-1," << elapsed << ",";  // build time (not applicable), overall time (== select time)
    log << get_num_keys() << "," << get_bytes_used() << ",";
    write_to_file(log.str());
}