
#ifndef UTILS_UTILS_HPP_
#define UTILS_UTILS_HPP_

#include <vector>
#include <algorithm>

template<class T, class U>
bool sorted_list_contains(const std::vector<T>& container, const U& v)
{
    auto it = std::lower_bound(
        container.begin(),
        container.end(),
        v,
        [](const T& l, const U& r){ return l < r; });
    return it != container.end() && *it == v;
}

#endif // UTILS_UTILS_HPP_