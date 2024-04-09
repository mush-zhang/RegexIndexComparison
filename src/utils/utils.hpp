
#ifndef UTILS_UTILS_HPP_
#define UTILS_UTILS_HPP_

#include <vector>
#include <algorithm>

template<class T, class U>
static bool sorted_list_contains(const std::vector<T>& container, const U& v)
{
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
    size_t i = 0, j = 0;
    std::vector<T> temp;
    while (i < l.size() && j < r.size()) {
        T candidate;
        if (l[i] < r[j]) {
            candidate = l[i++];
        } else {
            candidate = r[j++];
            if (l[i] == r[j]) i++;
        } 
        if (candidate != temp.back()) {
            temp.push_back(candidate);
        }
    } 
    // there can be at most one of l and r that has remaining elements
    if (i < l.size()) {
        if(l[i] != temp.back()) {
            temp.push_back(l[i++]);
        } 
        while (i < l.size()) {
            temp.push_back(l[i++]);
        }
    } 
    if (j < r.size()) {
        if (r[j] != temp.back()) {
            temp.push_back(r[j++]);
        } 
        while (j < r.size()) {
            temp.push_back(r[j++]);
        }
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