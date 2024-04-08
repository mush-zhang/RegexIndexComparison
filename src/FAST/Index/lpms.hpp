#ifndef FAST_INDEX_LPMS_INDEX_HPP_
#define FAST_INDEX_LPMS_INDEX_HPP_

#include "../../FREE/Index/multigram_index.hpp"

namespace fast_index {

class LpmsIndex : public NGramInvertedIndex {
 public:
    enum relaxation_type { k_deterministic, k_randomized, kInvalid };

    LpmsIndex() = delete;
    LpmsIndex(const LpmsIndex &&) = delete;
    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), 
        k_queries_(queries), k_queries_size_(queries.size),
        k_relaxation_type_(k_deterministic) {}

    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries,
              relaxation_type re_type)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), 
        k_queries_(queries), k_queries_size_(queries.size),
        k_relaxation_type_(re_type) {}
    
    ~LpmsIndex() {}

    void build_index(int upper_k) override;

 protected:
    void select_grams(int upper_k) override;
    
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
        const std::vector<std::vector<std::string>> & query_literals);

    template <typename T, class hash_T>
    std::vector<bool> build_model(size_t k,
        const std::unordered_map<size_t, long double> & r_count, 
        const std::unordered_map<size_t, long double> & q_count, 
        const std::vector<std::vector<size_t>> & qg_map);
    

};

} // namespace fast_index

#endif // FAST_INDEX_LPMS_INDEX_HPP_