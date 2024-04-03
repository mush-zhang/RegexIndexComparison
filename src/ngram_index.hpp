#ifndef NGRAM_INDEX_HPP_
#define NGRAM_INDEX_HPP_

#include <vector>
#include <string>
#include <set>

class NGramIndex {
 public:
    NGramIndex() = delete;
    NGramIndex(const NGramIndex &&) = delete;
    NGramIndex(const std::vector<std::string> & dataset)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()) {}
    
    ~NGramIndex() {}

    virtual void build_index(int upper_k) {}

    // return all substrings of the given string that are keys in the index
    virtual std::vector<std::string> find_all_indexed(const std::string & line) = 0;

    virtual void print_index() = 0;
    
    virtual const std::vector<unsigned int> & get_line_pos_at(const std::string & key) = 0;

    const std::vector<std::string> & get_dataset() const {
        return k_dataset_;
    }

 protected:
    virtual void select_grams(int upper_k) {};
    
    // the index structure should be stored here
    const std::vector<std::string> &k_dataset_;
    const unsigned int k_dataset_size_;

    std::set<std::string> k_index_keys_;
};

#endif // NGRAM_INDEX_HPP_