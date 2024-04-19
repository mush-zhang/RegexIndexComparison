
#ifndef UTILS_UTILS_HPP_
#define UTILS_UTILS_HPP_

#include <vector>
#include <algorithm>
#include <cassert>

#ifdef NDEBUG
#define assert(x) (void(0))
#endif

template <typename T>
size_t calculate_vector_size(const std::vector<T> & vec) {
    return sizeof(std::vector<T>) + (sizeof(T) * vec.size());
}

template<class T, class U>
static bool sorted_list_contains(const std::vector<T>& container, const U& v)
{
    // TODO remove assert after debugging
    // assert(std::is_sorted(container.cbegin(), container.cend()) && "list not sorted");
    auto it = std::lower_bound(
        container.begin(),
        container.end(),
        v,
        [](const T& l, const U& r){ return l < r; });
    return it != container.end() && *it == v;
}

template<class T, class U>
static std::vector<T> sorted_lists_union(const std::vector<T> & l, 
        const std::vector<U> & r) {
    // TODO remove assert after debugging
    // assert(std::is_sorted(l.cbegin(), l.cend()) && "Left list not sorted");
    // assert(std::is_sorted(r.cbegin(), r.cend()) && "Right list not sorted");
    size_t i = 0, j = 0;
    std::vector<T> temp;
    while (i < l.size() && j < r.size()) {
        T candidate;
        if (l[i] < r[j]) {
            candidate = l[i++];
        } else {
            candidate = r[j];
            if (l[i] == r[j]) i++;
            j++;
        } 
        if (temp.empty() || candidate != temp.back()) {
            temp.push_back(candidate);
        }
    } 
    // there can be at most one of l and r that has remaining elements
    if (i < l.size()) {
        if(l[i] != temp.back()) {
            temp.push_back(l[i++]);
        } 
        temp.insert(temp.end(), l.cbegin() + i, l.cend());
    } 
    if (j < r.size()) {
        if (r[j] != temp.back()) {
            temp.push_back(r[j++]);
        } 
        temp.insert(temp.end(), r.cbegin() + j, r.cend());
    }
    return temp;
}

template<class T, class U>
static std::vector<T> sorted_lists_intersection(const std::vector<T> & l, 
        const std::vector<U> & r) {
    size_t i = 0, j = 0;
    std::vector<T> temp;
    while (i < l.size() && j < r.size()) {
        if (l[i] < r[j]) {
            i++;
        } else if (l[i] > r[j]) {
            j++;
        } else {
            temp.push_back(l[i]);
            i++; 
            j++;
        }
    } 
    return temp;
}
#endif // UTILS_UTILS_HPP_