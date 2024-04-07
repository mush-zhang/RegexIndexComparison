#ifndef FAST_INDEX_LPMS_INDEX_HPP_
#define FAST_INDEX_LPMS_INDEX_HPP_

#include "../../FREE/Index/multigram_index.hpp"

namespace fast_index {

class LpmsIndex : public free_index::MultigramIndex {
 public:
    enum relaxation_type { k_deterministic, k_randomized, kInvalid };

    LpmsIndex() = delete;
    LpmsIndex(const LpmsIndex &&) = delete;
    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries)
      : NGramInvertedIndex(dataset, 1), 
        k_queries_(queries), k_queries_size_(queries.size),
        k_relaxation_type_(k_deterministic) {}

    LpmsIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries,
              relaxation_type re_type)
      : NGramInvertedIndex(dataset, 1), 
        k_queries_(queries), k_queries_size_(queries.size),
        k_relaxation_type_(re_type) {}
    
    ~LpmsIndex() {}

    void build_index(int upper_k) override;

 protected:
    void select_grams(int upper_k) override;
    
 private:
    const std::vector<std::string> & k_queries_;
    const long double k_queries_size_;
    const relaxation_type k_relaxation_type_;
};

} // namespace fast_index

#endif // FAST_INDEX_LPMS_INDEX_HPP_