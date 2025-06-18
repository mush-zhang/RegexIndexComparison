#ifndef TRIGRAM_INDEX_TRIGRAM_INVERTED_INDEX_HPP_
#define TRIGRAM_INDEX_TRIGRAM_INVERTED_INDEX_HPP_

#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <mutex>
#include <thread>
#include <algorithm>
#include "../../ngram_inverted_index.hpp"

namespace trigram_index {

class TrigramInvertedIndex : public NGramInvertedIndex {
 public:
    TrigramInvertedIndex() = delete;
    TrigramInvertedIndex(const TrigramInvertedIndex &&) = delete;

    TrigramInvertedIndex(const std::vector<std::string> & dataset)
        : NGramInvertedIndex(dataset) {}

    TrigramInvertedIndex(const std::vector<std::string> & dataset,
                         const std::vector<std::string> & queries)
        : NGramInvertedIndex(dataset, queries) {}

    ~TrigramInvertedIndex() {}

    void build_index(int upper_n = 3) override;

    bool empty() const override { return k_index_keys_.empty(); }

    size_t get_num_keys() const override { return k_index_keys_.size(); }

 protected:
    void extract_trigrams(const std::string & line, std::set<std::string> & trigrams) const;
    void fill_posting();

    std::mutex index_mutex_;
};

} // namespace trigram_index

#endif // TRIGRAM_INDEX_TRIGRAM_INVERTED_INDEX_HPP_