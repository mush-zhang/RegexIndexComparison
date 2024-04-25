#ifndef BITVEC_QUERY_MATCHER_HPP_
#define BITVEC_QUERY_MATCHER_HPP_

#include "simple_query_matcher.hpp"
#include "ngram_bitvec_index.hpp"

template<size_t N, size_t K> 
class BitvecQueryMatcher : public SimpleQueryMatcher {
 public:
    BitvecQueryMatcher() = delete;

    BitvecQueryMatcher(const NGramBitvecIndex<N,K> & index, 
                 const std::vector<std::string> & regs,
                 bool compile=true) 
                 : SimpleQueryMatcher(index, regs, compile) {}

    BitvecQueryMatcher(const NGramBitvecIndex<N,K> & index, bool compile=true) 
        : SimpleQueryMatcher(index, compile) {}

    ~BitvecQueryMatcher() {}

 protected:
    // const NGramIndex & k_index_;
    // std::unordered_map<std::string, std::shared_ptr<RE2>> reg_evals_;

    bool get_indexed(const std::string & reg, 
                     std::vector<size_t> & container) const override {
        return k_index_.get_all_idxs(reg, container);
    }
};

#endif // BITVEC_QUERY_MATCHER_HPP_