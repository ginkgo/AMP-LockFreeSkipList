/*
 * GlobalLockHeap.h
 *
 *  Created on: Nov 30, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef GLOBALLOCKHEAP_H_
#define GLOBALLOCKHEAP_H_

#include "../../../settings.h"

#include <pheet/primitives/PerformanceCounter/DummyPerformanceCounters.h>

#include <string.h>
#include <iostream>

namespace pheet {

/*
 * MaxGlobalLockHeap
 */
template <class Pheet, typename TT, class Comparator = std::less<TT> >
class GlobalLockHeap {
public:
	typedef DummyPerformanceCounters<Pheet> PerformanceCounters;

	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;
	typedef TT T;

	GlobalLockHeap();
	GlobalLockHeap(Comparator const& comp);
	~GlobalLockHeap();

	void push(T const& item);
	bool try_peek(T& ret);
	bool try_pop(T& ret);

	size_t size() const;

	static void print_name();

private:
	void bubble_up(size_t index);
	void bubble_down(size_t index);

	size_t capacity;
	std::atomic<size_t> length;
	TT* data;
	Comparator is_less;
	Mutex m;
};

template <class Pheet, typename TT, class Comparator>
GlobalLockHeap<Pheet, TT, Comparator>::GlobalLockHeap()
: capacity(4), length(0), data(new TT[capacity]) {

}

template <class Pheet, typename TT, class Comparator>
GlobalLockHeap<Pheet, TT, Comparator>::GlobalLockHeap(Comparator const& comp)
: capacity(4), length(0), data(new TT[capacity]), is_less(comp) {

}

template <class Pheet, typename TT, class Comparator>
GlobalLockHeap<Pheet, TT, Comparator>::~GlobalLockHeap() {
	delete[] data;
}

template <class Pheet, typename TT, class Comparator>
void GlobalLockHeap<Pheet, TT, Comparator>::push(T const& item) {
	LockGuard g(m);

	size_t l = size();
	if(l == capacity) {
		size_t new_capacity = capacity << 1;
		T* new_data = new T[new_capacity];
		memcpy(new_data, data, sizeof(T) * capacity);
		delete[] data;
		data = new_data;
		capacity = new_capacity;
	}

	data[l] = item;
	bubble_up(size());
	length.store(l + 1, std::memory_order_relaxed);
}

template <class Pheet, typename TT, class Comparator>
bool GlobalLockHeap<Pheet, TT, Comparator>::try_peek(TT& ret) {
	LockGuard g(m);

	if(size() == 0) {
		return false;
	}

	pheet_assert(size() > 0);
	ret = data[0];
	return true;
}


template <class Pheet, typename TT, class Comparator>
bool GlobalLockHeap<Pheet, TT, Comparator>::try_pop(TT& ret) {
	LockGuard g(m);

	size_t l = size();
	if(l == 0) {
		return false;
	}
	ret = data[0];
	length.store(l - 1, std::memory_order_relaxed);
	data[0] = data[l];
	bubble_down(0);
	return ret;
}

template <class Pheet, typename TT, class Comparator>
size_t GlobalLockHeap<Pheet, TT, Comparator>::size() const {
	return length.load(std::memory_order_relaxed);
}


template <class Pheet, typename TT, class Comparator>
void GlobalLockHeap<Pheet, TT, Comparator>::bubble_up(size_t index) {
	size_t next = (index - 1) >> 1;
	while(index > 0 && is_less(data[next], data[index])) {
		std::swap(data[next], data[index]);
		index = next;
		next = (index - 1) >> 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void GlobalLockHeap<Pheet, TT, Comparator>::bubble_down(size_t index) {
	size_t next = (index << 1) + 1;
	size_t l = size();
	while(next < l) {
		size_t nnext = next + 1;
		if(nnext < l && is_less(data[next], data[nnext])) {
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
void GlobalLockHeap<Pheet, TT, Comparator>::print_name() {
	std::cout << "GlobalLockHeap<";
	Mutex::print_name();
	std::cout << ">";
}

}

#endif /* GLOBALLOCKHEAP_H_ */
