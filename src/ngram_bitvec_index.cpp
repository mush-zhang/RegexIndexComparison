#include "ngram_bitvec_index.hpp"

#include "utils/reg_utils.hpp"
#include "utils/utils.hpp"

static const std::vector<size_t> k_empty_pos_list_;

template<size_t N, size_t K> 
size_t NGramBitvecIndex<N,K>::get_bytes_used() const {
    auto bucketSize = sizeof(void*);
    auto adminSize = 3 * sizeof(void*) + sizeof(size_t);
    size_t totalSize = 0;
    // if (max) {
    //     totalSize += adminSize + inv_idx.max_bucket_count() * bucketSize;
    // } else {
    //     totalSize += adminSize + inv_idx.bucket_count() * bucketSize;
    // } 
    totalSize += adminSize + k_index_keys_.bucket_count() * bucketSize;

    auto contentSize = 0;
    for (const auto & [key, val] : k_index_keys_) {
        // key size with ptr
        contentSize += N + sizeof(void*) + sizeof(size_t);
    }

    totalSize += contentSize;
    totalSize += calculate_vector_size(k_index_);
    return totalSize;
}

template<size_t N, size_t K> 
void NGramBitvecIndex<N,K>::print_index(bool size_only) const {
    std::cout << "size of dataset: " << k_dataset_size_;
    std::cout << ", size of keys: " << k_index_keys_.size();
    std::cout << ", size of index: " << k_index_.size() << std::endl;
    std::cout << ", size of index in bytes: " << get_bytes_used() << std::endl;
    for (const auto & [key, val] : k_index_keys_) {
        std::cout << "\"" << std::string(key.data(), N) << "\"" << ": ";
    }
    std::cout << std::endl;
}

template<size_t N, size_t K> 
const std::vector<size_t> & NGramBitvecIndex<N,K>::get_line_pos_at(
        const std::string & key) const { 
    std::cerr << "this should not be called really" << std::endl;
    return k_empty_pos_list_;
}

template<size_t N, size_t K> 
std::bitset<K> NGramBitvecIndex<N,K>::GetBitMask(const std::string & reg) const {
    // get the bit mask for the regex
    std::vector<std::string> reg_lits = extract_literals(reg);
    std::unordered_set<std::array<char,N>, hash_array> reg_ngrams{};
    for (const auto & lit : reg_lits) {
        reg_ngrams.merge(make_unique_ngrams<N>(lit));
    }
    std::bitset<K> bitmask;
    bitmask.set();
    // start with all 1's
    for (const auto & reg_gram: reg_ngrams) {
        auto it = k_index_keys_.find(reg_gram);
        if (it != k_index_keys_.end()) {
            // bigram exits, unset bit at the index
            bitmask.reset(it->second);
        }
    }
    // return a bitmask where the positions we want to tests are 0's, and others are 1's.
    return bitmask;
}

template<size_t N, size_t K> 
std::vector<std::string> NGramBitvecIndex<N,K>::find_all_keys(
        const std::string & reg) const {
    std::cerr << "should not be called during normal matching " << std::endl;

    std::vector<std::string> found_keys;
    std::vector<std::string> reg_lits = extract_literals(reg);
    std::unordered_set<std::array<char,N>, hash_array> reg_ngrams{};
    for (const auto & lit : reg_lits) {
        reg_ngrams.merge(make_unique_ngrams<N>(lit));
    }
    
    for (const auto & reg_gram: reg_ngrams) {
        auto it = k_index_keys_.find(reg_gram);
        if (it != k_index_keys_.end()) {
            found_keys.push_back(std::string(it->first.data(), N));
        }
    }
    return found_keys;
}

template<size_t N, size_t K> 
bool NGramBitvecIndex<N,K>::get_all_idxs(
        const std::string & reg, std::vector<size_t> & container) const {
        std::vector<std::string> found_keys;
    auto bitmask = GetBitMask(reg);
    if (bitmask.all()) {
        return false;
    }
    for (auto i = 0; i < k_dataset_size_; i++) {
        // check if the indexed ngrams in regular expressions are present in the current log line
        if ((k_index_[i] | bitmask).all()) {
            container.push_back(i);
        } 
    }
    return true;
}

template<size_t N, size_t K> 
void NGramBitvecIndex<N,K>::find_all_keys_helper(
        const std::string & line, std::vector<std::string> & found_keys) const {
    std::cerr << "SHould not be called" << std::endl;
}

template class NGramBitvecIndex<2, 8>;
template class NGramBitvecIndex<2, 64>;
template class NGramBitvecIndex<3, 4>;
template class NGramBitvecIndex<4, 4>;

// template std::unordered_set<std::array<char,2>, hash_array> make_unique_ngrams(const std::string& s);
