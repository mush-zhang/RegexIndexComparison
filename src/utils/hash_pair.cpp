#include "hash_pair.hpp"
#include <functional>

// struct hash_pair hash_pair;

template <typename T>
void hash_combine (size_t& seed, const T& val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// auxiliary generic functions to create a hash value using a seed
template <typename T, typename... Types>
void hash_combine(size_t& seed, const T& val, const Types&... args) {
    hash_combine(seed,val);
    hash_combine(seed,args...);
}

// optional auxiliary generic functions to support hash_val() without arguments
void hash_combine (size_t& seed) {}

//  generic function to create a hash value out of a heterogeneous list of arguments
template <typename... Types>
size_t hash_val(const Types&... args) {
    size_t seed = 0;
    hash_combine(seed, args...);
    return seed;
}

template <class T1, class T2>
size_t hash_pair::operator()(const std::pair<T1, T2>& p) const
{
    return hash_val(p.first, p.second);
}

template <typename... T>    
std::size_t hash_tuple::operator()(const std::tuple<T...>& p) const
{
    return std::apply(hash_val<T...>, p);
}

template <typename T, size_t n>    
std::size_t hash_array::operator()(const std::array<T, n>& p) const
{
    size_t seed = 0;
    for (size_t i = 0; i < n; i++) {
        hash_combine(seed, p[i]);
    }
    return seed;
}

template size_t hash_pair::operator()<char>(const std::pair<char, char>& p) const;

template size_t hash_tuple::operator()<char>(const std::tuple<char, char, char>& p) const;
template size_t hash_tuple::operator()<char>(const std::tuple<char, char>& p) const;

template size_t hash_array::operator()<char>(const std::array<char, 4>& p) const;
template size_t hash_array::operator()<char>(const std::array<char, 3>& p) const;
template size_t hash_array::operator()<char>(const std::array<char, 2>& p) const;

// auto b = std::tuple_cat(arr);
