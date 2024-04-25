#ifndef REI_INDEX_FREQ_NGRAM_INDEX_HPP_
#define REI_INDEX_FREQ_NGRAM_INDEX_HPP_

#include "../../ngram_bitvec_index.hpp"

namespace rei_index {

template<size_t N, size_t K> 
class FreqNgramIndex : public NGramBitvecIndex<N, K> {
 public:
    FreqNgramIndex() = delete;
    FreqNgramIndex(const FreqNgramIndex &&) = delete;
    FreqNgramIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries)
      : NGramBitvecIndex<N,K>(dataset, queries), k_threshold_(1) {}

	FreqNgramIndex(const std::vector<std::string> & dataset, 
              const std::vector<std::string> & queries,
			  double sel_threshold)
      : NGramBitvecIndex<N,K>(dataset, queries), k_threshold_(sel_threshold) {}
    
    ~FreqNgramIndex() {}

    void build_index(int upper_n=-1) override;

 private:
    void select_grams(int upper_n=-1) override;

	void fill_posting();

	/** The selectivity of the gram in index will be <= k_threshold_**/
    const double k_threshold_;
};

} // namespace rei_index

#endif // REI_INDEX_FREQ_NGRAM_INDEX_HPP_