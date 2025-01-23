#pragma once 
#include <cstddef>

size_t RoundUpToPowerOfTwo(size_t size) {
	// Rounds up the size to the nearest higher power of two 
	size--;
	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	size |= size >> 32;
	size++;
	return size;
}
