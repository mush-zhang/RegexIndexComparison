#ifndef FREE_INDEX_PLAIN_MULTIGRAM_HPP_
#define FREE_INDEX_PLAIN_MULTIGRAM_HPP_

#include <vector>
#include <string>
#include <unordered_set>

namespace free {

class PlainMultigram {
 public:
    PlainMultigram() = delete;
    PlainMultigram(const std::vector<std::string> &&) = delete;
    PlainMultigram(const std::vector<std::string> & dataset, double sel_threshold)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()), k_threshold_(sel_threshold) {}
    ~PlainMultigram();

    void build_index(int upper_k);


 private:
    // the index structure should be stored here
    const std::vector<std::string> &k_dataset_;
    const long double k_dataset_size_;
    const double k_threshold_;
    std::unordered_map<std::string, std::vector<long>> k_index_;
    std::unordered_set<std::string, std::vector<long>> k_index_keys_;

    void select_grams(int upper_k);

    /**Select Grams Helpers**/
    void get_kgrams_not_indexed(
            std::unordered_set<std::string> & kgrams,
            const std::unordered_set<std::string> & expand, size_t k);
    void get_uni_bigram(
        std::unordered_map<char, long> & unigrams,
        std::unordered_map<std::pair<char, char>, long, hash_pair> & bigrams);

    void insert_kgram_into_index(
        const std::unordered_map<std::string, long double> & kgrams,
        std::unordered_set<std::string> & expand);
    void insert_uni_bigram_into_index(
        const std::unordered_map<char, long double> & unigrams,
        const std::unordered_map<std::pair<char, char>, long double, hash_pair> & bigrams,
        std::unordered_set<std::string> & expand);
    /**Select Grams Helpers End**/
};

} // namespace free

#endif // FREE_INDEX_PLAIN_MULTIGRAM_HPP_