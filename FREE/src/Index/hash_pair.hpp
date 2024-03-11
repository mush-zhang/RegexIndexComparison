// ------------------------------------------------------------------------------------------------------
//  Tuple Hash Functions
//  Hashing strategy of tuples: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
// ------------------------------------------------------------------------------------------------------

struct hash_pair {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const;
};