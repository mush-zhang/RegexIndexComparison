#include "ngram_btree_index.hpp"
#include "utils/utils.hpp"

static const std::vector<size_t> k_empty_pos_list_;

long long int NGramBtreeIndex::get_bytes_used() const { 
    long long int contentSize = k_index_.bytes_used() + k_index_keys_.bytes_used(); 
    for (const auto & [key, val] : k_index_) {
        // value size
        contentSize += calculate_vector_size(val);
    }
    return contentSize;
};

void NGramBtreeIndex::print_index(bool size_only) const {
    std::cout << "size of dataset: " << k_dataset_size_;
    std::cout << ", size of keys: " << k_index_keys_.size();
    std::cout << ", size of index: " << k_index_.size() << std::endl;
    std::cout << ", size of index in bytes: " << k_index_.bytes_used() << std::endl;
    for (const auto & key : k_index_keys_) {
        std::cout << "\"" << key << "\"" << ": ";
        if (size_only) {
            std::cout << k_index_.at(key).size() << " lines" << std::endl;
        } else {
            std::cout << "[";
            for (auto idx : k_index_.at(key)) {
                std::cout << idx << ",";
            }
            std::cout << "]"  << std::endl;
        }
    }
}

const std::vector<size_t> & NGramBtreeIndex::get_line_pos_at(
        const std::string & key) const { 
    if (auto it = k_index_.find(key); it != k_index_.end()) {
        return it->second;
    }
    return k_empty_pos_list_;
}

std::vector<std::string> NGramBtreeIndex::find_all_keys(
        const std::string & reg) const {
    std::vector<std::string> found_keys;
    auto literals = extract_literals(reg);
    for (const auto & lit : literals) {
        find_all_keys_helper(lit, found_keys);
    }
    return found_keys;
}

void NGramBtreeIndex::find_all_keys_helper(
        const std::string & line, std::vector<std::string> & found_keys) const {
    for (size_t i = 0; i < line.size(); i++) {
        auto curr_c = line.at(i);
        std::string curr_key = line.substr(i,1);
        auto lower_it = k_index_keys_.lower_bound(curr_key);

        for (auto & it = lower_it; 
             it != k_index_keys_.end() && curr_key.at(0) == curr_c; 
             ++it) {
            // check if the current key is the same with curren substr
            curr_key = *it;
            if (curr_key == line.substr(i, curr_key.size())) {
                found_keys.emplace_back(curr_key);
                // break as it is a prefix free set
                break;
            }
        }
    }
}