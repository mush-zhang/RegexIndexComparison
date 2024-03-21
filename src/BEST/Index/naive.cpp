#include "naive.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include "../../utils/reg_utils.hpp"

// Algorithm 2 in Figure 3
void best_index::NaiveIndex::build_index(int upper_k) {
    // auto start = std::chrono::high_resolution_clock::now();
    select_grams(upper_k);
    // auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
    //     std::chrono::high_resolution_clock::now() - start).count();
    // std::cout << "Select Grams End in " << elapsed << " s" << std::endl;

    // start = std::chrono::high_resolution_clock::now();
    // fill_posting(upper_k);
    // elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
    //     std::chrono::high_resolution_clock::now() - start).count();
    // std::cout << "Index Building End in " << elapsed << std::endl;
}

void best_index::NaiveIndex::print_index() {
    std::cout << "size of dataset: " << k_dataset_size_;
    std::cout << ", size of keys: " << k_index_keys_.size();
    std::cout << ", size of index: " << k_index_.size() << std::endl;
    for (const auto & key : k_index_keys_) {
        std::cout << key << ": ";
        std::cout << "[";
        for (auto idx : k_index_[key]) {
            std::cout << idx << ",";
        }
        std::cout << "]"  << std::endl;
    }
}

/** In c++20; we probably not going to use it
    As S, M1 are sparse

    #include <iostream>
    #include <vector>
    #include <ranges>

    int main() {
        std::vector<int> a = {1, 2, 3};
        std::vector<int> b = {10, 20};

        auto cartesian_view = std::views::cartesian_product(a, b);

        for (const auto& pair : cartesian_view) {
            int result = std::get<0>(pair) * std::get<0>(pair) + std::get<1>(pair) * std::get<1>(pair);
            std::cout << "Result: " << result << std::endl;
        }

        return 0;
    }
**/

int best_index::NaiveIndex::insert_gram(const std::string & l) {
    char cl[l.size()+1];
    strcpy(cl, l.c_str());
    unsigned char* ucl = reinterpret_cast<unsigned char*>(cl);
    return raxTryInsert(gram_tree_, &ucl[0], l.size(), NULL, NULL);
}

std::vector<std::string> best_index::NaiveIndex::generate_path_labels() {
    std::vector<std::string> path_labels;
    raxIterator iter;
    raxStart(&iter, gram_tree_);
    raxSeek(&iter,"^",NULL,0);

    while(raxNext(&iter)) {
        std::string curr((char*)iter.key, (int)iter.key_len);
        path_labels.push_back(curr);
    }
    raxStop(&iter);
    return path_labels;
}

/**Described in section 5: Candidate Set Generation
 * Note: it generate the set of prefixes of the set of all suffixes of the string set
 *       which isn't essentially the set of all substrings/multigrams.
 *       is it due to storage consideration?
 *       Anyway, we are storing the set of all multigrams in a map and record their count
 *       so that we tranverse the dataset only once
 *       Or else if we generate the prefix set for each single suffix on the fly
 *       it will take several scans on the dataset; will only be fine if dataset is small
 **/
std::unordered_set<std::string> best_index::NaiveIndex::candidate_gram_set_gen() {
    std::unordered_set<std::string> result;
    // 1. Build suffix tree using all queries
    for (const auto & q : k_queries_) {
        std::vector<std::string> literals = extract_literals(q);
        for (const auto & l : literals) {
            for (size_t i = 0; i < l.size(); i++) {
                insert_gram(l.substr(i, l.size() - i));
            }
        }
    }
    // 2. get all path labels in sorted list I guess
    auto path_labels = generate_path_labels();

    // 3. get all prefixes of path labels; 
    //    store them in an ordered map
    std::map<std::string, unsigned int> pre_suf_count;
    std::string prev = "";
    for (const auto & pl : path_labels) {
        for (size_t i = 0; i < pl.size(); i++) {
            // get all prefixes
            pre_suf_count.insert({pl.substr(0, i+1), 0});
        }
    }

    // 3.5 iterate once on the dataset and 
    //     count the number of occurrance of each multigram
    int threshold_count = k_threshold_ * k_dataset_size_;
    // Note:
    // intially I thought of doing: if count > threshold count, then erase the key
    for (const auto & line : k_dataset_) {
        for (size_t i = 0; i < line.size(); i++) {
            auto curr_c = line.at(i);
            std::string curr_key = line.substr(i,1);
            auto lower_it = pre_suf_count.lower_bound(curr_key);

            for (auto & it = lower_it; it != pre_suf_count.end() && curr_key.at(0) == curr_c; ++it) {
                // check if the current key is the same with curren substr
                curr_key = it->first;
                if (curr_key == line.substr(i, curr_key.size())) {
                    pre_suf_count[curr_key]++;
                }
            }
        }
    }

    // 4. Get the smallest prefix of the remaining grams with
    //    selectivity less than c
    std::string prev_key = "";
    for (const auto & [key, val] : pre_suf_count) {
        if (val > threshold_count) continue;
        if (prev_key.empty() ||                           // first to insert
            prev_key != key.substr(0, prev_key.size())) { // or prev not prefix
            
            prev_key = key;
            k_index_keys_.insert(key);
        }
    }

    return result;
}

// Algorithm 2 in Figure 3
// Basic greedy gram selection algorightm
void best_index::NaiveIndex::select_grams(int upper_k) {
    /** referring to the implementation detail in section 2.2
        Example 3 and the paragraph above **/
    auto candidates = candidate_gram_set_gen();
    // Generate M2
    
}

std::vector<std::string> best_index::NaiveIndex::find_all_indexed(const std::string & line) {
    std::vector<std::string> found_keys;
    // for (size_t i = 0; i < line.size(); i++) {
    //     auto curr_c = line.at(i);
    //     std::string curr_key = line.substr(i,1);
    //     auto lower_it = k_index_keys_.lower_bound(curr_key);

    //     for (auto & it = lower_it; it != k_index_keys_.end() && curr_key.at(0) == curr_c; ++it) {
    //         // check if the current key is the same with curren substr
    //         curr_key = *it;
    //         if (curr_key == line.substr(i, curr_key.size())) {
    //             found_keys.emplace_back(curr_key);
    //             // break as it is a prefix free set
    //             break;
    //         }
    //     }
    // }
    return found_keys;
}

