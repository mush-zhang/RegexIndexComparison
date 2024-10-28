
#ifndef UTILS_UTILS_HPP_
#define UTILS_UTILS_HPP_

#include <vector>
#include <unordered_set>
#include <array>
#include <cassert>

#include "hash_pair.hpp"

#ifdef NDEBUG
#define assert(x) (void(0))
#endif

template <typename T>
size_t calculate_vector_size(const std::vector<T> & vec);

template<class T, class U>
static bool sorted_list_contains(const std::vector<T>& container, const U& v);

template<class T, class U>
static std::vector<T> sorted_lists_union(const std::vector<T> & l, 
        const std::vector<U> & r);

template<class T, class U>
static std::vector<T> sorted_lists_intersection(const std::vector<T> & l, 
        const std::vector<U> & r);

template<size_t N> 
static void insert_unique_ngrams_to_set(
        std::unordered_set<std::array<char,N>, hash_array> & ngrams, const std::string & s);

template<size_t N> 
static std::unordered_set<std::array<char,N>, hash_array> 
make_unique_ngrams(const std::string& s);

// Randomly select sampleSize number of items from 0 to rangeUpperBound
// code from here: https://stackoverflow.com/a/28287837
static std::unordered_set<int> BobFloydAlgo(int sampleSize, int rangeUpperBound)
{
    std::unordered_set<int> sample;
    // std::random_device rd;
    // std::mt19937 generator(rd());
    std::default_random_engine generator;

    for(int d = rangeUpperBound - sampleSize; d < rangeUpperBound; d++)
    {
        int t = uniform_int_distribution<>(0, d)(generator);
        if (sample.find(t) == sample.end())
            sample.insert(t);
        else
            sample.insert(d);
    }
    return sample;
}

#endif // UTILS_UTILS_HPP_