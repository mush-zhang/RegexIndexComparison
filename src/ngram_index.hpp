#ifndef NGRAM_INDEX_HPP_
#define NGRAM_INDEX_HPP_

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>

#include "utils/reg_utils.hpp"

class NGramIndex {
 public:
    NGramIndex() = delete;
    NGramIndex(const NGramIndex &&) = delete;
    NGramIndex(const std::vector<std::string> & dataset)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()),
	  	k_queries_(std::vector<std::string>()), k_queries_size_(0) {}
    
	NGramIndex(const std::vector<std::string> & dataset,
			  const std::vector<std::string> & queries)
      : k_dataset_(dataset), k_dataset_size_(dataset.size()),
	  	k_queries_(queries), k_queries_size_(queries.size()) {}

    ~NGramIndex() {}

    virtual void build_index(int upper_k) {}

    // return all substrings of the given string that are keys in the index
    virtual std::vector<std::string> find_all_indexed(const std::string & line) const = 0;

    virtual void print_index(bool size_only=false) const = 0;
    
    virtual const std::vector<size_t> & get_line_pos_at(const std::string & key)  const = 0;

    const std::vector<std::string> & get_dataset() const {
        return k_dataset_;
    }

	const std::vector<std::string> & get_queries() const {
        return k_queries_;
    }

    std::vector<std::vector<std::string>> get_query_literals() const {
		std::vector<std::vector<std::string>> query_literals;
		for (const auto & q : k_queries_) {
			std::vector<std::string> literals = extract_literals(q);
			query_literals.push_back(literals);
		}
		return query_literals;
	}
    
    virtual bool empty() const = 0;

    virtual size_t get_num_keys() const = 0;

    virtual size_t get_bytes_used() const = 0;

    void set_outfile(std::ofstream & outfile) { outfile_ = &outfile; }

 protected:
    virtual void select_grams(int upper_k) {};
    
    // the index structure should be stored here
    const std::vector<std::string> & k_dataset_;
    const size_t k_dataset_size_;

	const long double k_queries_size_;
    const std::vector<std::string> & k_queries_;

    std::ostream* outfile_ = &std::cout;
    int thread_count_ = 1;
};

#endif // NGRAM_INDEX_HPP_