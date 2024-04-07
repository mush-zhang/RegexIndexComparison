#include "gurobi_c++.h"

#include "lpms.hpp"


static const k_deterministic_threshold_ = 0.99;

using gram_set = std::unordered_set<std::string>;

void fast_index::LpmsIndex::generate_children_set(
        const gram_set & expand, gram_set & children_set, size_t k) {

    // Important Note: This seems only effective if it is dealing with bio dataset
    //     which has small sized alphabet. The removal would require scanning the
    //     whole dataset after all, we might well just insert all occurred suffixes

    for (const auto & l : k_dataset_) {

    }
}

/**
 * @brief Apply LPMS-D or LPMS-R to find the vector x
 * 
 * @param r_count Key is gram_idx, value is the #records with this gram
 * @param q_count Key is gram_idx, value is the #queries with this gram
 * @param qg_map Key is query_idx, value is the vector of all gram_idxs in the query
 * @return * void 
 */
void fast_index::LpmsIndex::build_model(
        const std::unordered_map<size_t, unsigned int> & r_count, 
        const std::unordered_map<size_t, unsigned int> & q_count, 
        const std::unordered_map<size_t, std::vector<size_t>> & qg_map) {
    GRBEnv* env = 0;
    GRBVar* x = 0;
    GRBVar** mtx_A = 0;

    size_t num_grams = r_count.size();
    size_t num_queries = qg_map.size();

    std::vector<std::reference_wrapper<std::string>> grams_ordered;
    grams_ordered.reserve(num_grams);

    try {
        // Model
        env = new GRBEnv();
        GRBModel model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "model");

        // minimize \sum_{g \in G} c_g x_g ; x \in {0, 1}
        // Gurobi by default minimize the objective
        x = model.addVars(num_grams, GRB_CONTINUOUS);
        size_t g_idx = 0;
        for (const auto & [gram, curr_r_count] : r_count) {
            grams_ordered.push_back(gram);
            double c_g = curr_r_count / double(gram.size() * q_count[gram]);
            x[g_idx].set(GRB_DoubleAttr_Obj, c_g);
            x[g_idx].set(GRB_StringAttr_VarName, "x_" + std::to_string(g_idx));
            g_idx++;
        }

        // size_t g_idx;
        // for (g_idx = 0; g_idx < num_grams; ++g_idx) {
        //     double c_g = r_count[g_idx] / (k * q_count[g_idx]);
        //     x[g_idx].set(GRB_DoubleAttr_Obj, c_g);
        //     x[g_idx].set(GRB_StringAttr_VarName, "x_" + std::to_string(g_idx));
        // }

        // qg_map : key: q; value: set of g in q
        // subject to A
        size_t q_idx;
        size_t g_inner_idx;
        for (const auto & [query, curr_grams_in_q] : qg_map) {
            unsigned int b_q = 0;
            for (const auto & curr_gram : curr_grams_in_q) {
                if (r_count[curr_gram] > b_q) {
                    b_q = r_count[curr_gram];
                }
            }
            GRBLinExpr Ax_q = 0;
            // This should follow the order of the temp vector grams_ordered!
            for (g_inner_idx = 0; g_inner_idx < num_grams; ++g_inner_idx) {
                unsigned_int A_qg = 0;
                if (qg_map[q_idx].contains(grams_ordered[g_inner_idx])) {
                    A_qg = r_count[g_inner_idx];
                }
                Ax_q.addTerm(A_qg, x[g_inner_idx]);
            }
            model.addConstr(Ax_q >= b_q, "Ax_" + std::to_string(q_idx));
        }



        // size_t q_idx;
        // size_t g_inner_idx;
        // for (q_idx = 0; q_idx < num_queries; ++q_idx) {
        //     unsigned int b_q = 0;
        //     for (const auto & g_in_q : qg_map[q_idx]) {
        //         if (r_count[g_in_q] > b_q) {
        //             b_q = r_count[g_in_q];
        //         }
        //     }
        //     GRBLinExpr Ax_q = 0;
        //     for (g_inner_idx = 0; g_inner_idx < num_grams; ++g_inner_idx) {
        //         unsigned_int A_qg = 0;
        //         if (qg_map[q_idx].contains(g_inner_idx)) {
        //             A_qg = r_count[g_inner_idx];
        //         }
        //         Ax_q.addTerm(A_qg, x[g_inner_idx]);
        //     }
        //     model.addConstr(Ax_q >= b_q, "Ax_" + std::to_string(q_idx));
        // }

        // Solve
        model.optimize();

        std::vector<bool> x_result(num_grams, false);
        switch (k_relaxation_type_) {
            case k_deterministic:
                for (g_idx = 0; g_idx < num_grams; ++g_idx) {
                    if (x[g_idx].get(GRB_DoubleAttr_X) > k_deterministic_threshold_) {
                        x_result[g_idx] = true;
                    }
                }
                break;
            case k_randomized:
                for (g_idx = 0; g_idx < num_grams; ++g_idx) {
                    x_result[g_idx] = (std::rand() % 100) < x[g_idx].get(GRB_DoubleAttr_X);
                }
                break;
        }

        model.dispose();
        env.dispose();

    } catch (GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    } catch (Exception e2) {
        std::cout << "Exception during optimization" << std::endl;
        std::cout << e2.getMessage() << std::endl;
    }

    delete env;

}

void fast_index::LpmsIndex::uni_bi_special(std::unordered_set<std::string> & expand) {
    std::unordered_map<char, long double> unigrams;
    std::unordered_map<std::pair<char, char>, long double, hash_pair> bigrams;
    get_uni_bigram(unigrams, bigrams);

    // 3. build matrix A for unigram


    insert_uni_bigram_into_index(unigrams, bigrams, expand);
}

// Algorithm 1: LPMS multigram selection algorithm
void fast_index::LpmsIndex::select_grams(int upper_k) {
    // Initialize a empty expand set
    gram_set expand; // stores useless prefix

    // 1. for each multigram in expand set, 
    //    append a member in alphabet and save to children set

    // 2. remove all multigrams g in children set that
    //    for all k, g \notin CandidateSet(k, q)
    // Note: I guess like the gram g is not in any query

    // Note: special casing the unigrams and bigrams


    do {
        
    }


    // 3. populate matrix A, where A_{q, g} denote 
    //    r_count of g if g \in q; A has size num_query * num_gram
    // Note: Definition of A in Section 3.1, formulae (2)
    // Note: so whenever some gram g is in some query q, 
    //    the value of the matrix entry is the count of 
    //    number of records in the whole dataset that 
    //    contains this gram

    // 4. populate vector b, where b_q denote the smallest
    //    r_count among all grams in query q; b length num_query
    // Note: Definition of b in Section 3.1, formulae (3)

    // 5. populate vector c where c_g is 
    //    (r_count of g) / (|g| * q_count of g) 
    //    c length num_gram
    // Note: Definition of c in Section 3.1, formulae (5)

    // 6. use lp solver with deterministic relaxation 
    //    or random rounding to find x

    // 7. Move all multigtams in the children set
    //    whose associated value in x is 1 to G

    // 8. Those multigrams remainig become the new expand set

    // 9. stop until expand set empty.
}

void fast_index::LpmsIndex::build_index(int upper_k) {
    select_grams();
}