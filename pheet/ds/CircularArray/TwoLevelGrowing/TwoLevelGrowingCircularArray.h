/*
 * TwoLevelGrowingCircularArray.h
 *
 *  Created on: 12.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TWOLEVELGROWINGCIRCULARARRAY_H_
#define TWOLEVELGROWINGCIRCULARARRAY_H_

#include <pheet/settings.h>

#include <pheet/misc/bitops.h>

namespace pheet {

template <class Pheet, typename TT, size_t MaxBuckets = 32>
class TwoLevelGrowingCircularArrayImpl {
public:
	typedef TT T;

	template<size_t NewVal>
	using WithMaxBuckets = TwoLevelGrowingCircularArrayImpl<Pheet, TT, NewVal>;

	TwoLevelGrowingCircularArrayImpl();
	TwoLevelGrowingCircularArrayImpl(size_t initial_capacity);
	~TwoLevelGrowingCircularArrayImpl();

	size_t get_capacity();
	bool is_growable();

	// return value NEEDS to be const. When growing we cannot guarantee that the reference won't change
	T const& get(size_t i);
	void put(size_t i, T value);

	void grow(size_t bottom, size_t top);

private:
	const size_t initial_buckets;
	size_t buckets;
	size_t capacity;
	T* data[MaxBuckets];
};

template <class Pheet, typename T, size_t MaxBuckets>
TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::TwoLevelGrowingCircularArrayImpl()
: initial_buckets(5), buckets(initial_buckets), capacity(1 << (buckets - 1)) {
	pheet_assert(buckets <= MaxBuckets);

	T* ptr = new T[capacity];
	data[0] = ptr;
	ptr++;
	for(size_t i = 1; i < buckets; i++)
	{
		data[i] = ptr;
		ptr += 1 << (i - 1);
	}
}

template <class Pheet, typename T, size_t MaxBuckets>
TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::TwoLevelGrowingCircularArrayImpl(size_t initial_capacity)
: initial_buckets(find_last_bit_set(initial_capacity - 1) + 1), buckets(initial_buckets), capacity(1 << (buckets - 1)) {
	pheet_assert(initial_capacity > 0);
	pheet_assert(buckets <= MaxBuckets);

	T* ptr = new T[capacity];
	data[0] = ptr;
	ptr++;
	for(size_t i = 1; i < buckets; i++)
	{
		data[i] = ptr;
		ptr += 1 << (i - 1);
	}
}

template <class Pheet, typename T, size_t MaxBuckets>
TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::~TwoLevelGrowingCircularArrayImpl() {
	delete[] (data[0]);
	for(size_t i = initial_buckets; i < buckets; i++)
		delete [](data[i]);
}

template <class Pheet, typename T, size_t MaxBuckets>
size_t TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::get_capacity() {
	return capacity;
}

template <class Pheet, typename T, size_t MaxBuckets>
bool TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::is_growable() {
	return buckets < MaxBuckets;
}

// return value NEEDS to be const. When growing we cannot guarantee that the reference won't change
template <class Pheet, typename T, size_t MaxBuckets>
inline T const& TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::get(size_t i) {
	i = i % capacity;
	size_t hb = find_last_bit_set(i);
	return data[hb][i ^ ((1 << (hb)) >> 1)];
}

template <class Pheet, typename T, size_t MaxBuckets>
void TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::put(size_t i, T value) {
	i = i % capacity;
	size_t hb = find_last_bit_set(i);
	data[hb][i ^ ((1 << (hb)) >> 1)] = value;
}

template <class Pheet, typename T, size_t MaxBuckets>
void TwoLevelGrowingCircularArrayImpl<Pheet, T, MaxBuckets>::grow(size_t bottom, size_t top) {
	pheet_assert(is_growable());

	data[buckets] = new T[capacity];
	buckets++;
	size_t newCapacity = capacity << 1;

	size_t start = top;
	size_t startMod = top % capacity;
	if(startMod == (top % newCapacity))
	{	// Make sure we don't iterate through useless indices
		start += capacity - startMod;
	}
	for(size_t i = start; i < bottom; i++) {
		size_t oldI = i % capacity;
		size_t newI = i % newCapacity;
		if(oldI != newI)
		{
			size_t oldBit = find_last_bit_set(oldI);
			size_t newBit = find_last_bit_set(newI);
			data[newBit][newI ^ ((1 << (newBit)) >> 1)] =
				data[oldBit][oldI ^ ((1 << (oldBit)) >> 1)];
		}
		else
		{	// Make sure we don't iterate through useless indices
			break;
		}
	}

	// Do we really need this? I guess we have to ensure that threads don't have
	// some senseless pointers in their data array
	MEMORY_FENCE ();
	capacity = newCapacity;
}

template<class Pheet, typename TT>
using TwoLevelGrowingCircularArray = TwoLevelGrowingCircularArrayImpl<Pheet, TT>;


}

#endif /* TWOLEVELGROWINGCIRCULARARRAY_H_ */
