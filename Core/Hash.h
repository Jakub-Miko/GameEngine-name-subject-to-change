#pragma once
#include <functional>

template <class T>
inline void hash_combine(std::size_t& hash_v, const T& value)
{
	std::hash<T> hash;
	hash_v ^= hash(value) + 0x9e3779b9 + (hash_v << 6) + (hash_v >> 2);
}