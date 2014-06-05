/*
 * MergeHeap.h
 *
 *  Created on: Mar 6, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef MERGEHEAP_H_
#define MERGEHEAP_H_

#include "../../../settings.h"
#include <pheet/misc/bitops.h>

#include <iostream>

namespace pheet {

template <class TT>
struct MergeHeapSortedArray {
	typedef MergeHeapSortedArray<TT> Self;

	MergeHeapSortedArray()
	:filled(0),
	 next(nullptr)
	 {}
	TT* data;
	size_t size;
	size_t filled;
	Self* next;
/*	uint8_t default_slot;
	uint8_t current_slot;
	uint8_t filled_slot_id;*/
};


template <class Pheet, typename TT, class Comparator = std::less<TT> >
class MergeHeap {
public:
	typedef MergeHeap<Pheet, TT, Comparator> Self;
	typedef TT T;
	typedef MergeHeapSortedArray<TT> SA;

	template <class NP, typename NT, class NComparator = std::less<TT> >
	using BT = MergeHeap<NP, NT, NComparator>;

	MergeHeap()
	:filled(0), max(nullptr) {
		initialize();
	}
	MergeHeap(Comparator const& comp)
	:is_less(comp), filled(0), max(nullptr) {
		initialize();
	}
	~MergeHeap() {
		pheet_assert(empty());

		for(int i = 11; i < 126; ++i) {
			if(arrays[i].data != nullptr)
				delete[] arrays[i].data;
		}
	}

	void push(T item) {
		++filled;
		if(work->filled == 0) {
			work->data[0] = item;
			work->filled = 1;
			if(max == nullptr) {
				max = work;
			}
			else {
				pheet_assert(max != work);
				pheet_assert(max->filled > 0);
				if(is_less(max->data[max->filled - 1], item)) {
					max = work;
				}
			}
		}
		else {
			if(work->filled == 1) {
				if(is_less(item, work->data[0])) {
					work->data[1] = work->data[0];
					work->data[0] = item;
				}
				else {
					work->data[1] = item;
					if(max != work && is_less(max->data[max->filled - 1], item)) {
						max = work;
					}
				}
				work->filled = 2;
				if(work->size == 2) {
					put_work();
				}
			}
			else {
				if(is_less(item, work->data[work->filled - 1])) {
					// Put the existing work array into the data-structure and retrieve new work array
					put_work();
					// Not many checks required now. Item is obviously not max, and new work array should be empty
					pheet_assert(work->filled == 0);
					work->data[0] = item;
					work->filled = 1;
				}
				else {
					pheet_assert(work->filled < work->size);

					work->data[work->filled] = item;
					++(work->filled);
					if(max != work && is_less(max->data[max->filled - 1], item)) {
						max = work;
					}
					if(work->filled == work->size) {
						// We have completely filled up our work array. Put it into the data-structure and retrieve new work array
						put_work();
					}
				}
			}
		}
	}
	T peek() {
		pheet_assert(max != nullptr);
		pheet_assert(max->filled > 0);
		return max->data[max->filled - 1];
	}
	T pop() {
		pheet_assert(max != nullptr);
		pheet_assert(max->filled > 0);

		--filled;

		--max->filled;
		TT ret = max->data[max->filled];
		if(filled == 0) {
			max = nullptr;
			work = last_initialized_array;
			work->next = nullptr;
			pheet_assert(work->filled == 0);
		}
		else {
			if(work->filled >= 2 && work->next != nullptr && work->next->filled <= work->filled) {
				put_work();
			}

			SA* iter = work;
			SA* prev = work;

			if(iter->filled != 0) {
				max = iter;
			}
			else {
				max = nullptr;
			}

			while(iter->next != nullptr) {
				if(iter->next->filled == 0) {
					iter->next = iter->next->next;
				}
				else if(prev != iter && iter->next->filled <= ((iter->filled *3) >> 1)) {
					// work array never gets merged unless it's not the work array any more
					SA* tmp = merge(iter);
					pheet_assert(tmp != prev);
					pheet_assert(tmp->next != tmp);
					prev->next = tmp;
					iter = prev;
				}
				else {
					prev = iter;
					iter = iter->next;
					if(max == nullptr || is_less(max->data[max->filled - 1], iter->data[iter->filled - 1])) {
						max = iter;
					}
				}
			}
		}

		// TODO: what happens if it gets smaller than the slot limit?
/*		if(old_max->filled == 0) {
			slots[old_max->current_slot] = nullptr;
			--num_filled_slots;
			if(old_max->filled_slot_id != num_filled_slots) {
				filled_slots[num_filled_slots]->filled_slot_id = old_max->filled_slot_id;
				filled_slots[old_max->filled_slot_id] = filled_slots[num_filled_slots];
			}
		}*/

		return ret;
	}

	size_t get_length() const {
		return filled;
	}

	bool is_empty() const {
		return filled == 0;
	}
	inline bool empty() const {
		return is_empty();
	}

	static void print_name() {
		std::cout << "MergeHeap";
	}

private:
	SA* merge(SA* item) {
		pheet_assert(item->next != nullptr);
		if(item->next->filled == 0) {
			item->next = item->next->next;
			return item;
		}
		size_t total_size = item->filled + item->next->filled;
		SA* result = find_slot(total_size);
		result->next = item->next->next;

		size_t a = 0, b = 0, c = 0;
		while(a < item->filled && b < item->next->filled) {
			if(is_less(item->data[a], item->next->data[b])) {
				result->data[c] = item->data[a];
				++a;
			}
			else {
				result->data[c] = item->next->data[b];
				++b;
			}
			++c;
		}
		while(a < item->filled) {
			result->data[c] = item->data[a];
			++a;
			++c;
		}
		while(b < item->next->filled) {
			result->data[c] = item->next->data[b];
			++b;
			++c;
		}
		if(max == item || max == item->next) {
			max = result;
		}
		item->filled = 0;
		item->next->filled = 0;
		result->filled = c;
		return result;
	}

	void put_work() {
		while(work->next != nullptr && work->next->filled <= work->filled) {
			work = merge(work);
		}
		SA* old_work = work;
		work = find_slot(work->filled);

		work->next = old_work;
	}

	SA* find_slot(size_t length) {
		pheet_assert(length >= 2);
		int slot = (find_last_bit_set(length - 1) - 1) << 1;
		pheet_assert(slot >= 0);
		while(arrays[slot].filled != 0 || work == arrays + slot) {
			++slot;
		}
		if(arrays[slot].data == nullptr) {
			last_initialized_array = arrays + slot;

			size_t size = 1 << ((slot >> 1) + 1);
			pheet_assert(size >= length);
			arrays[slot].size = size;
			arrays[slot].data = new T[size];
		}
		pheet_assert(arrays[slot].size >= length);
		return arrays + slot;
	}

	void initialize() {
		TT* data = new TT[254];
		for(int i = 11; i >= 0; --i) {
			int slot = (i >> 1);
			arrays[i].data = data;
			size_t size = 1 << (slot + 1);
			arrays[i].size = size;
		//	arrays[i].default_slot = slot;
			data += size;
		}
		work = arrays + 11;
		last_initialized_array = work;
		for(int i = 12; i < 126; ++i) {
			arrays[i].data = nullptr;
			arrays[i].filled = 0;
		}
/*		for(int i = 0; i < 63; ++i) {
			slots[i] = nullptr;
		}*/
	}

	Comparator is_less;

	size_t filled;
	SA* last_initialized_array;
//	size_t num_filled_slots;
	SA* work;
	SA* max;

//	SA* slots[63];
//	SA* filled_slots[63];
	SA arrays[126];
};

} /* namespace pheet */
#endif /* MERGEHEAP_H_ */
