/*
 * Heap.h
 *
 *  Created on: Nov 30, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef HEAP_H_
#define HEAP_H_

#include "../../../settings.h"

#include <string.h>
#include <iostream>

namespace pheet {

/*
 * MaxHeap
 */
template <class Pheet, typename TT, class Comparator = std::less<TT> >
class Heap {
public:
	typedef TT T;
	template <class NP, typename NT, class NComparator = std::less<TT> >
	using BT = Heap<NP, NT, NComparator>;

	Heap();
	Heap(Comparator const& comp);
	~Heap();

	void push(T item);
	T peek();
	T pop();

	size_t get_length() const;
	bool is_empty() const;
	inline bool empty() const { return is_empty(); }

	static void print_name();

private:
	void bubble_up(size_t index);
	void bubble_down(size_t index);

	size_t capacity;
	size_t length;
	TT* data;
	Comparator is_less;
};

template <class Pheet, typename TT, class Comparator>
Heap<Pheet, TT, Comparator>::Heap()
: capacity(4), length(0), data(new TT[capacity]) {

}

template <class Pheet, typename TT, class Comparator>
Heap<Pheet, TT, Comparator>::Heap(Comparator const& comp)
: capacity(4), length(0), data(new TT[capacity]), is_less(comp) {

}

template <class Pheet, typename TT, class Comparator>
Heap<Pheet, TT, Comparator>::~Heap() {
	delete[] data;
}

template <class Pheet, typename TT, class Comparator>
void Heap<Pheet, TT, Comparator>::push(T item) {
	if(length == capacity) {
		size_t new_capacity = capacity << 1;
		T* new_data = new T[new_capacity];
		memcpy(new_data, data, sizeof(T) * capacity);
		delete[] data;
		data = new_data;
		capacity = new_capacity;
	}

	data[length] = item;
	bubble_up(length);
	++length;
}

template <class Pheet, typename TT, class Comparator>
TT Heap<Pheet, TT, Comparator>::peek() {
	pheet_assert(length > 0);
	return data[0];
}


template <class Pheet, typename TT, class Comparator>
TT Heap<Pheet, TT, Comparator>::pop() {
	pheet_assert(length > 0);
	T ret = data[0];
	--length;
	data[0] = data[length];
	bubble_down(0);
	return ret;
}

template <class Pheet, typename TT, class Comparator>
size_t Heap<Pheet, TT, Comparator>::get_length() const {
	return length;
}

template <class Pheet, typename TT, class Comparator>
bool Heap<Pheet, TT, Comparator>::is_empty() const {
	return length == 0;
}

template <class Pheet, typename TT, class Comparator>
void Heap<Pheet, TT, Comparator>::bubble_up(size_t index) {
	size_t next = (index - 1) >> 1;
	while(index > 0 && is_less(data[next], data[index])) {
		std::swap(data[next], data[index]);
		index = next;
		next = (index - 1) >> 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void Heap<Pheet, TT, Comparator>::bubble_down(size_t index) {
	size_t next = (index << 1) + 1;
	while(next < length) {
		size_t nnext = next + 1;
		if(nnext < length && is_less(data[next], data[nnext])) {
			next = nnext;
		}
		if(!is_less(data[index], data[next])) {
			break;
		}
		std::swap(data[index], data[next]);
		index = next;
		next = (index << 1) + 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void Heap<Pheet, TT, Comparator>::print_name() {
	std::cout << "Heap";
}

}

#endif /* HEAP_H_ */
