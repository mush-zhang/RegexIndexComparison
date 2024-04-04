#include <iostream>
#include "ngram_btree_index.hpp"

static const std::vector<unsigned int> k_empty_pos_list_;

entry_key_t NGramBtreeIndex::iter_to_key(std::set<std::string>::iterator & it) {
    return reinterpret_cast<entry_key_t>(std::to_address(it));
}

const std::vector<unsigned int> & NGramBtreeIndex::get_line_pos_at(
        const std::string & key) {
    auto it = k_index_keys_.find(key); 
    if (it == k_index_keys_.end()) return k_empty_pos_list_;
    
    auto addr = k_index_.btree_search(iter_to_key(it));
    if (!addr) return k_empty_pos_list_;
    auto result = reinterpret_cast<size_t>(addr);
    return k_idx_lists_[result-1];
}

void NGramBtreeIndex::print_index() {
    std::cout << "size of dataset: " << k_dataset_size_;
    std::cout << ", size of keys: " << k_index_keys_.size();
    std::cout << ", size of index: " << std::endl;
    k_index_.printAll();
    std::cout << "Printing actual value" << std::endl;

    for (const auto & key : k_index_keys_) {
        std::cout << key << ": [" << std::flush;;

        auto idxs = get_line_pos_at(key);
        for (auto idx : idxs) {
            std::cout << idx << ",";
        }
        std::cout << "]"  << std::endl;
    }
}

std::vector<std::string> NGramBtreeIndex::find_all_indexed(
        const std::string & line) {
    std::vector<std::string> found_keys;
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
    return found_keys;
}

