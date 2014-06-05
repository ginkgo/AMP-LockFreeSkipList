/*
 * FixedSizeCircularArray.h
 *
 *  Created on: 12.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef FIXEDSIZECIRCULARARRAY_H_
#define FIXEDSIZECIRCULARARRAY_H_

#include "../../../settings.h"

namespace pheet {

template <class Pheet, typename T>
class FixedSizeCircularArray {
public:
	FixedSizeCircularArray(size_t initial_capacity);
	~FixedSizeCircularArray();

	size_t get_capacity();
	bool is_growable();

	T& get(size_t i);
	void put(size_t i, T value);

	void grow(size_t bottom, size_t top);
private:
	size_t const capacity;
	T* data;
};

template <class Pheet, typename T>
FixedSizeCircularArray<Pheet, T>::FixedSizeCircularArray(size_t initial_capacity)
: capacity(initial_capacity) {
	data = new T[capacity];
}

template <class Pheet, typename T>
FixedSizeCircularArray<Pheet, T>::~FixedSizeCircularArray() {
	delete[] data;
}

template <class Pheet, typename T>
size_t FixedSizeCircularArray<Pheet, T>::get_capacity() {
	return capacity;
}

template <class Pheet, typename T>
bool FixedSizeCircularArray<Pheet, T>::is_growable() {
	return false;
}

template <class Pheet, typename T>
T& FixedSizeCircularArray<Pheet, T>::get(size_t i) {
	return data[i % capacity];
}

template <class Pheet, typename T>
void FixedSizeCircularArray<Pheet, T>::put(size_t i, T item) {
	data[i % capacity] = item;
}

template <class Pheet, typename T>
void FixedSizeCircularArray<Pheet, T>::grow(size_t, size_t) {
	pheet_assert(false);
}

}

#endif /* FIXEDSIZECIRCULARARRAY_H_ */
