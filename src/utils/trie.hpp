#ifndef FREE_INDEX_TRIE_HPP_
#define FREE_INDEX_TRIE_HPP_

#include <string>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/trie_policy.hpp>
#include <ext/pb_ds/tag_and_trait.hpp>

// using namespace __gnu_pbds;

typedef __gnu_pbds::null_type mapped_type;
typedef __gnu_pbds::trie_string_access_traits<> cmp_fn;
typedef __gnu_pbds::pat_trie_tag tag_type;

typedef __gnu_pbds::trie<std::string, mapped_type, cmp_fn, tag_type, 
        __gnu_pbds::trie_prefix_search_node_update> trie_type;


// // This is for when k_index_keys_ is implemented using trie
// std::vector<std::string> free_index::MultigramIndex::find_all_indexed(const std::string line) {
//     std::vector<std::string> found_keys;
//     for (size_t i = 0; i < line.size(); i++) {
//         auto match_range = k_index_keys_.prefix_range(line.substr(i,1));
//         for (const auto & it = match_range.first; it != match_range.second; ++it) {
//             // check if the current key is the same with curren substr
//             const std::string & curr_key = *it;
//             if (curr_key == line.substr(i, curr_key.size())) {
//                 found_keys.emplace_back(curr_key);
//                 // break as it is a prefix free set
//                 break;
//             }
//         }
//     }
// }

#endif // FREE_INDEX_TRIE_HPP_