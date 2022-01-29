#ifndef HASH_FEATURE_TABLE_HPP
#define HASH_FEATURE_TABLE_HPP
#include <vector>
#include <string>
#include <sstream>
namespace HashFeatureTable {
template<typename T>
inline size_t HashCombine(size_t key, const T& value) {
     std::hash<T> hash_func;
      return key ^ (hash_func(value) + 0x9e3779b9 + (key << 6) + (key >> 2));
}
template<>
inline size_t HashCombine<size_t>(size_t key, const size_t& value) {
    return key ^ (value + 0x9e3779b9 + (key << 6) + (key >> 2));
}
}
#endif
