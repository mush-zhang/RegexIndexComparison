#ifndef FREE_INDEX_HASH_PAIR_HPP_
#define FREE_INDEX_HASH_PAIR_HPP_

#include <cstddef>
#include <utility>
#include <array>

// ------------------------------------------------------------------------------------------------------
//  Tuple Hash Functions
//  Hashing strategy of tuples: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
// ------------------------------------------------------------------------------------------------------

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const;
};

struct hash_tuple {
    template <typename... T>    
    std::size_t operator()(const std::tuple<T...>& p) const;
};

struct hash_array {
    template <typename T, size_t n>    
    std::size_t operator()(const std::array<T, n>& p) const;
};

#endif // FREE_INDEX_HASH_PAIR_HPP_