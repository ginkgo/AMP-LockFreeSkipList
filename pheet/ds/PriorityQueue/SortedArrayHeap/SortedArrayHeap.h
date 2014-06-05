/*
 * SortedArrayHeap.h
 *
 *  Created on: Nov 30, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SORTEDARRAYHEAP_H_
#define SORTEDARRAYHEAP_H_

#include "../../../settings.h"
#include <iostream>

namespace pheet {

template <typename T>
struct SortedArrayHeapArray {
	T* data;
	size_t start;
	size_t end;
	size_t capacity;
	size_t max_heap_pos;
	size_t min_heap_pos;
};

template <class Pheet, typename TT, class Comparator = std::less<TT> >
class SortedArrayHeap {
public:
	typedef TT T;

	SortedArrayHeap();
	SortedArrayHeap(Comparator const& comp);
	~SortedArrayHeap();

	void push(T item);
	T peek();
	T pop();
	T peek_min();
	T pop_min();

	size_t get_length() const;
	bool is_empty() const;

	static void print_name();

private:
	bool max_heap_put(size_t index, T item);
	bool min_heap_put(size_t index, T item);

	void bubble_up_max(size_t index);
	void bubble_up_min(size_t index);
	void bubble_down_max(size_t index);
	void bubble_down_min(size_t index);

	void grow(SortedArrayHeapArray<TT>* el);

	size_t capacity;
	size_t heap_length;
	size_t length;
	SortedArrayHeapArray<TT>** max_heap;
	SortedArrayHeapArray<TT>** min_heap;
	Comparator is_less;

	SortedArrayHeapArray<TT>* reuse_el;
};


template <class Pheet, typename TT, class Comparator>
SortedArrayHeap<Pheet, TT, Comparator>::SortedArrayHeap()
: capacity(4), heap_length(0), length(0), max_heap(new SortedArrayHeapArray<TT>*[capacity]), min_heap(new SortedArrayHeapArray<TT>*[capacity]), reuse_el(NULL) {

}

template <class Pheet, typename TT, class Comparator>
SortedArrayHeap<Pheet, TT, Comparator>::SortedArrayHeap(Comparator const& comp)
: capacity(4), heap_length(0), length(0), max_heap(new SortedArrayHeapArray<TT>*[capacity]), min_heap(new SortedArrayHeapArray<TT>*[capacity]), is_less(comp), reuse_el(NULL) {

}

template <class Pheet, typename TT, class Comparator>
SortedArrayHeap<Pheet, TT, Comparator>::~SortedArrayHeap() {
	if(reuse_el != NULL) {
		delete[] reuse_el->data;
		delete reuse_el;
	}
	for(size_t i = 0; i < heap_length; ++i) {
		delete[] max_heap[i]->data;
		delete max_heap[i];
	}
	delete[] max_heap;
	delete[] min_heap;
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::push(T item) {
	if(heap_length > 0) {
		size_t pos = heap_length - 1;
		// Try adding to existing heap
		if(max_heap_put(pos, item)) {
			return;
		}
		else if(min_heap_put(pos, item)) {
			return;
		}
		// Try adding to parents of potential new element
		pos >>= 1;
		if(max_heap_put(pos, item)) {
			return;
		}
		else if(min_heap_put(pos, item)) {
			return;
		}
	}

	// No existing heap elements available - create a new one
	if(heap_length == capacity) {
		size_t new_capacity = capacity << 1;
		SortedArrayHeapArray<TT>** new_heap = new SortedArrayHeapArray<TT>*[new_capacity];
		memcpy(new_heap, max_heap, sizeof(SortedArrayHeapArray<TT>*) * capacity);
		delete[] max_heap;
		max_heap = new_heap;

		new_heap = new SortedArrayHeapArray<TT>*[new_capacity];
		memcpy(new_heap, min_heap, sizeof(SortedArrayHeapArray<TT>*) * capacity);
		delete[] min_heap;
		min_heap = new_heap;

		capacity = new_capacity;
	}

	SortedArrayHeapArray<T>* el;
	if(reuse_el != NULL) {
		el = reuse_el;
		reuse_el = NULL;
	}
	else {
		el = new SortedArrayHeapArray<T>;
		el->data = new T[4];
		el->capacity = 4;
	}
	el->start = std::numeric_limits<size_t>::max() >> 1;
	el->end = el->start + 1;
	el->data[el->start % el->capacity] = item;
	el->max_heap_pos = heap_length;
	el->min_heap_pos = heap_length;
	max_heap[heap_length] = el;
	min_heap[heap_length] = el;

	++heap_length;
	++length;
	// No bubbling needed, because otherwise we could have just added to the arrays
}

template <class Pheet, typename TT, class Comparator>
TT SortedArrayHeap<Pheet, TT, Comparator>::peek() {
	SortedArrayHeapArray<T>* el = max_heap[0];
	return el->data[el->start % el->capacity];
}


template <class Pheet, typename TT, class Comparator>
TT SortedArrayHeap<Pheet, TT, Comparator>::pop() {
	SortedArrayHeapArray<T>* el = max_heap[0];
	T ret = el->data[el->start % el->capacity];

	++(el->start);
	if(el->start == el->end) {
		size_t min_pos = el->min_heap_pos;
		if(reuse_el == NULL) {
			reuse_el = el;
		}
		else {
			delete[] el->data;
			delete el;
		}
		--heap_length;
		if(heap_length != 0) {
			max_heap[0] = max_heap[heap_length];
			max_heap[0]->max_heap_pos = 0;
			bubble_down_max(0);
			if(min_pos != heap_length) {
				min_heap[min_pos] = min_heap[heap_length];
				min_heap[min_pos]->min_heap_pos = min_pos;
				bubble_down_min(min_pos);
			}
		}
	}
	else {
		bubble_down_max(0);
	}

	--length;
	return ret;
}

template <class Pheet, typename TT, class Comparator>
TT SortedArrayHeap<Pheet, TT, Comparator>::peek_min() {
	SortedArrayHeapArray<T>* el = min_heap[0];
	return el->data[el->start % el->capacity];
}


template <class Pheet, typename TT, class Comparator>
TT SortedArrayHeap<Pheet, TT, Comparator>::pop_min() {
	SortedArrayHeapArray<T>* el = min_heap[0];
	T ret = el->data[el->start % el->capacity];

	++(el->start);
	if(el->start == el->end) {
		size_t max_pos = el->max_heap_pos;
		if(reuse_el == NULL) {
			reuse_el = el;
		}
		else {
			delete[] el->data;
			delete el;
		}
		--heap_length;
		if(heap_length != 0) {
			if(max_pos != heap_length) {
				max_heap[max_pos] = max_heap[heap_length];
				max_heap[max_pos]->max_heap_pos = max_pos;
				bubble_down_max(max_pos);
			}
			min_heap[0] = min_heap[heap_length];
			min_heap[0]->min_heap_pos = 0;
			bubble_down_min(0);
		}
	}
	else {
		bubble_down_min(0);
	}

	--length;
	return ret;
}

template <class Pheet, typename TT, class Comparator>
size_t SortedArrayHeap<Pheet, TT, Comparator>::get_length() const {
	return length;
}

template <class Pheet, typename TT, class Comparator>
bool SortedArrayHeap<Pheet, TT, Comparator>::is_empty() const {
	return length == 0;
}

template <class Pheet, typename TT, class Comparator>
bool SortedArrayHeap<Pheet, TT, Comparator>::max_heap_put(size_t index, T item) {
	size_t valid = index;
	SortedArrayHeapArray<TT>* el = max_heap[index];

	while(is_less(el->data[el->start % el->capacity], item)) {
		valid = index;
		index = (index - 1) >> 1;
		if(valid == 0) {
			break;
		}
		el = max_heap[index];
	}

	if(valid == index) {
		return false;
	}
	++length;
	el = max_heap[valid];
	if(el->end - el->start == el->capacity) {
		grow(el);
	}
	--(el->start);
	el->data[el->start % el->capacity] = item;
	return true;
}

template <class Pheet, typename TT, class Comparator>
bool SortedArrayHeap<Pheet, TT, Comparator>::min_heap_put(size_t index, T item) {
	size_t valid = index;
	SortedArrayHeapArray<TT>* el = min_heap[index];

	while(is_less(item, el->data[(el->end - 1) % el->capacity])) {
		valid = index;
		index = (index - 1) >> 1;
		if(valid == 0) {
			break;
		}
		el = min_heap[index];
	}

	if(valid == index) {
		return false;
	}
	++length;
	el = min_heap[valid];
	if(el->end - el->start == el->capacity) {
		grow(el);
	}
	el->data[el->end % el->capacity] = item;
	++(el->end);
	return true;
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::bubble_up_max(size_t index) {
	size_t next = (index - 1) >> 1;
	SortedArrayHeapArray<TT>* el = max_heap[index];
	SortedArrayHeapArray<TT>* nel = max_heap[next];

	while(index > 0 && is_less(el->data[el->start % el->capacity], nel->data[nel->start % nel->capacity])) {
		swap(max_heap[next], max_heap[index]);
		el->max_heap_pos = next;
		nel->max_heap_pos = index;
		index = next;
		next = (index - 1) >> 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::bubble_up_min(size_t index) {
	size_t next = (index - 1) >> 1;
	SortedArrayHeapArray<TT>* el = min_heap[index];
	SortedArrayHeapArray<TT>* nel = min_heap[next];

	while(index > 0 && is_less(nel->data[(nel->end - 1) % nel->capacity], el->data[(el->end - 1) % el->capacity])) {
		swap(min_heap[next], min_heap[index]);
		el->min_heap_pos = next;
		nel->min_heap_pos = index;
		index = next;
		next = (index - 1) >> 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::bubble_down_max(size_t index) {
	size_t next = (index << 1) + 1;
	while(next < heap_length) {
		size_t nnext = next + 1;
		SortedArrayHeapArray<TT>* nel = max_heap[next];
		T nel_item = nel->data[nel->start % nel->capacity];
		if(nnext < heap_length) {
			SortedArrayHeapArray<TT>* nnel = max_heap[nnext];
			T nnel_item = nnel->data[nnel->start % nnel->capacity];
			if(is_less(nel_item, nnel_item)) {
				next = nnext;
				nel = nnel;
				nel_item = nnel_item;
			}
		}

		SortedArrayHeapArray<TT>* el = max_heap[index];
		if(!is_less(el->data[el->start % el->capacity], nel_item)) {
			break;
		}
		std::swap(max_heap[index], max_heap[next]);
		el->max_heap_pos = next;
		nel->max_heap_pos = index;
		index = next;
		next = (index << 1) + 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::bubble_down_min(size_t index) {
	size_t next = (index << 1) + 1;
	while(next < heap_length) {
		size_t nnext = next + 1;
		SortedArrayHeapArray<TT>* nel = min_heap[next];
		T nel_item = nel->data[(nel->end - 1) % nel->capacity];
		if(nnext < heap_length) {
			SortedArrayHeapArray<TT>* nnel = min_heap[nnext];
			T nnel_item = nnel->data[(nnel->end - 1) % nnel->capacity];
			if(is_less(nnel_item, nel_item)) {
				next = nnext;
				nel = nnel;
				nel_item = nnel_item;
			}
		}

		SortedArrayHeapArray<TT>* el = min_heap[index];
		if(!is_less(nel_item, el->data[(el->end - 1) % el->capacity])) {
			break;
		}
		std::swap(min_heap[index], min_heap[next]);
		el->min_heap_pos = next;
		nel->min_heap_pos = index;
		index = next;
		next = (index << 1) + 1;
	}
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::grow(SortedArrayHeapArray<TT>* el) {
	size_t new_capacity = el->capacity << 1;
	T* new_data = new T[new_capacity];

	for(size_t i = el->start; i != el->end; ++i) {
		new_data[i % new_capacity] = el->data[i % el->capacity];
	}
	delete[] el->data;
	el->data = new_data;
	el->capacity = new_capacity;
}

template <class Pheet, typename TT, class Comparator>
void SortedArrayHeap<Pheet, TT, Comparator>::print_name() {
	std::cout << "SortedArrayHeap";
}

}

#endif /* SORTEDARRAYHEAP_H_ */
