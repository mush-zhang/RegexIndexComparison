#ifndef FAST_INDEX_LPMS_INDEX_HPP_
#define FAST_INDEX_LPMS_INDEX_HPP_

#include "gurobi_c++.h"

#include "../../ngram_inverted_index.hpp"

namespace fast_index {

enum relaxation_type { kDeterministic, kRandomized, kInvalid };

class LpmsIndex : public NGramInvertedIndex {
 public:

    LpmsIndex() = delete;
    LpmsIndex(const LpmsIndex &&) = delete;

    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries,
              relaxation_type re_type = kDeterministic)
      : NGramInvertedIndex(dataset, queries),
        k_relaxation_type_(re_type) {
            thread_count_ = 1;
        }

    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries,
              int thread_count, 
              relaxation_type re_type = kDeterministic)
      : LpmsIndex(dataset, queries, re_type) {
            thread_count_ = thread_count;
        }
    
    ~LpmsIndex() {}

    void build_index(int upper_n=-1) override;

 protected:
    void select_grams(int upper_n=-1) override;
    
 private:
    const relaxation_type k_relaxation_type_;

    void get_kgrams_r(std::unordered_map<size_t, long double> & r_count,
        std::unordered_map<std::string, size_t> & kgrams,
        std::vector<std::vector<size_t>> & gr_map,
        const std::unordered_set<std::string> & expand, size_t k);

    void get_unigram_r(std::unordered_map<size_t, long double> & uni_count,
        std::unordered_map<char, size_t> & unigrams,
        std::vector<std::vector<size_t>> & uni_gr_map);

    void uni_special(std::unordered_set<std::string> & expand, 
        const std::vector<std::vector<std::string>> & query_literals, 
        GRBEnv * env);

    std::vector<bool> build_model(size_t k,
        const std::unordered_map<size_t, long double> & r_count, 
        const std::unordered_map<size_t, long double> & q_count, 
        const std::vector<std::set<size_t>> & qg_map, GRBEnv * env);    

};

} // namespace fast_index

#endif // FAST_INDEX_LPMS_INDEX_HPP_