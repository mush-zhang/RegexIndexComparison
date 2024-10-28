#include <algorithm>
#include <random>

#include "utils.hpp"
#include "hash_pair.hpp"

template <typename T>
size_t calculate_vector_size(const std::vector<T> & vec) {
    return sizeof(std::vector<T>) + (sizeof(T) * vec.size());
}

template<class T, class U>
static bool sorted_list_contains(const std::vector<T>& container, const U& v)
{
    assert(std::is_sorted(container.cbegin(), container.cend()) && "list not sorted");
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
    assert(std::is_sorted(l.cbegin(), l.cend()) && "Left list not sorted");
    assert(std::is_sorted(r.cbegin(), r.cend()) && "Right list not sorted");
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
    assert(std::is_sorted(l.cbegin(), l.cend()) && "Left list not sorted");
    assert(std::is_sorted(r.cbegin(), r.cend()) && "Right list not sorted");
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

template<size_t N> 
static void insert_unique_ngrams_to_set(
        std::unordered_set<std::array<char,N>, hash_array> & ngrams, const std::string & s) {
    if (s.size() >= N) {
        const auto end_iter = s.end() - (N - 1);
        for(auto it = s.cbegin(); it != end_iter; ++it) {
            std::array<char, N> temp;
            std::copy_n(it, N, temp.begin());
            ngrams.insert(temp);
        }
    }
}

template<size_t N> 
static std::unordered_set<std::array<char,N>, hash_array> 
make_unique_ngrams(const std::string& s) {
    std::unordered_set<std::array<char,N>, hash_array> ngrams;
    insert_unique_ngrams_to_set<N>(ngrams, s);
    return ngrams;
}

// Randomly select sampleSize number of items from 0 to rangeUpperBound
// code from here: https://stackoverflow.com/a/28287837
std::unordered_set<int> BobFloydAlgo(int sampleSize, int rangeUpperBound)
{
    std::unordered_set<int> sample;
    // std::random_device rd;
    // std::mt19937 generator(rd());
    std::default_random_engine generator;

    for(int d = rangeUpperBound - sampleSize; d < rangeUpperBound; d++)
    {
        int t = std::uniform_int_distribution<>(0, d)(generator);
        if (sample.find(t) == sample.end())
            sample.insert(t);
        else
            sample.insert(d);
    }
    return sample;
}