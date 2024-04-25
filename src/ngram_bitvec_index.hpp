#ifndef NGRAM_BITVEC_INDEX_HPP_
#define NGRAM_BITVEC_INDEX_HPP_

#include <bitset>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

#include "utils/hash_pair.hpp"
#include "ngram_index.hpp"

template<size_t N, size_t K> 
class NGramBitvecIndex : public NGramIndex {
 public:
    NGramBitvecIndex() = delete;
    NGramBitvecIndex(const NGramBitvecIndex &&) = delete;
    
    // NGramBitvecIndex(const std::vector<std::string> & dataset, 
    //         const std::vector<std::string> & queries) : 
    //         k_dataset_(dataset), k_dataset_size_(dataset.size()),
    //         k_queries_(queries), k_queries_size_(dataset.size()) {
    //     k_index_.reserve(dataset.size());
    //     k_index_keys_.reserve(K);
    // }

    NGramBitvecIndex(const std::vector<std::string> & dataset, 
            const std::vector<std::string> & queries) : 
            NGramIndex(dataset, queries) {
        k_index_.reserve(dataset.size());
        k_index_keys_.reserve(K);
    }

    ~NGramBitvecIndex() {}

    void build_index(int upper_n=-1) override {}

    std::vector<std::string> find_all_keys(const std::string & line) const override;

    void print_index(bool size_only=false) const override;
    
    const std::vector<size_t> & get_line_pos_at(const std::string & key) const override;

    // const std::vector<std::string> & get_dataset() const {
    //     return k_dataset_;
    // }

    // size_t get_dataset_size() const { return k_dataset_size_; }

	// const std::vector<std::string> & get_queries() const {
    //     return k_queries_;
    // }

    bool empty() const override { return k_index_keys_.empty(); }

    size_t get_num_keys() const override { return k_index_keys_.size(); }

    size_t get_bytes_used() const override;

    bool get_all_idxs(const std::string & reg, std::vector<size_t> & container) const;

    // void set_thread_count(int thread_count) { thread_count_ = thread_count; }

    // void set_outfile(std::ofstream & outfile) { outfile_ = &outfile; }

    // void write_to_file(const std::string & str) const { *outfile_ << str; }  

 protected:
    void select_grams(int upper_n=-1) override {}

    std::vector<std::bitset<K>> k_index_;
    std::unordered_map<std::array<char,N>, size_t, hash_array> k_index_keys_;
    
    // // the index structure should be stored here
    // const std::vector<std::string> & k_dataset_;
    // const size_t k_dataset_size_;

	// const long double k_queries_size_;
    // const std::vector<std::string> & k_queries_;

    // int thread_count_ = 1;

  private:
    // std::ostream* outfile_ = &std::cout;

    void find_all_keys_helper(
        const std::string & line,  std::vector<std::string> & found_keys) const;

    std::bitset<K> GetBitMask(const std::string & reg_string) const;
};

#endif // NGRAM_BITVEC_INDEX_HPP_