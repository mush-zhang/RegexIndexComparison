#include "hash_pair.hpp"

struct hash_pair hash_pair;

template <typename T>
void hash_combine (size_t& seed, const T& val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// auxiliary generic functions to create a hash value using a seed
template <typename T, typename... Types>
void hash_combine (size_t& seed, const T& val, const Types&... args) {
    hash_combine(seed,val);
    hash_combine(seed,args...);
}

// optional auxiliary generic functions to support hash_val() without arguments
void hash_combine (size_t& seed) {}

//  generic function to create a hash value out of a heterogeneous list of arguments
template <typename... Types>
size_t hash_val (const Types&... args) {
    size_t seed = 0;
    hash_combine(seed, args...);
    return seed;
}

template <class T1, class T2>
size_t hash_pair::operator()(const std::pair<T1, T2>& p) const
{
    return hash_val(p.first, p.second);
}