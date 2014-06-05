/*
 * MergeStrategyItemHeap.h
 *
 *  Created on: Mar 8, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef MERGESTRATEGYITEMHEAP_H_
#define MERGESTRATEGYITEMHEAP_H_

#include "MergeStrategyHeapHeap.h"

#include <pheet/misc/bitops.h>


namespace pheet {


template <class TT>
struct MergeStrategyItemHeapSortedArray {
	typedef MergeStrategyItemHeapSortedArray<TT> Self;

	MergeStrategyItemHeapSortedArray()
	:filled(0),
	 dead(0),
	 next(nullptr)
	 {}
	TT* data;
	size_t size;
	size_t filled;
	size_t dead;
	Self* next;
/*	uint8_t default_slot;
	uint8_t current_slot;
	uint8_t filled_slot_id;*/
};

template <class Pheet, typename T, class BaseStrategy, class Strategy, class StrategyRetriever>
class MergeStrategyItemHeap : public MergeStrategyHeapBase<Pheet, T> {
public:
	typedef MergeStrategyHeapBase<Pheet, T> Base;
	typedef MergeStrategyItemHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever> Self;
	typedef MergeStrategyItemHeapSortedArray<T> SA;

	MergeStrategyItemHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:comp(sr), sr(sr), filled(0), max(nullptr) {
		Base*& base = heap_heaps[std::type_index(typeid(Strategy))];
		if(base == nullptr) {
			pheet_assert(std::type_index(typeid(Strategy)) != std::type_index(typeid(BaseStrategy)));
			base = new MergeStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>(sr, heap_heaps);
		}
		parent = static_cast<MergeStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>*>(base);

		T* data = new T[254];
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
	}

	~MergeStrategyItemHeap() {
		pheet_assert(is_empty());

		for(int i = 11; i < 126; ++i) {
			if(arrays[i].data != nullptr)
				delete[] arrays[i].data;
		}
	}

	bool is_empty() const {
		return filled == 0;
	}

	T pop() {
		pheet_assert(max != nullptr);
		pheet_assert(max->filled > 0);
		T ret;
		pheet_assert(!comp(this->top, max->data[max->filled - 1]) && !comp(max->data[max->filled - 1], this->top));

		--filled;
		if(max->dead > 0) {
			pheet_assert(filled > 0);

			--(max->dead);
			ret = max->data[max->filled + max->dead];
			return ret;
		}

		--(max->filled);
		// Test ordering property
		pheet_assert(max->filled == 0 || !comp(max->data[max->filled], max->data[max->filled - 1]));
//		pheet_assert(this->top.block == max->data[max->filled].block && this->top.index == max->data[max->filled].index);
		ret = this->top;

		if(filled == 0) {
			max = nullptr;
			work = last_initialized_array;
			work->next = nullptr;
			pheet_assert(work->filled == 0);

			parent->pop_index(this->parent_index);
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
					pheet_assert(iter->next->dead == 0);
					iter->next = iter->next->next;
				}
				else if(prev != iter && (iter->next->filled + iter->next->dead) <= (((iter->filled + iter->dead) *3) >> 1)) {
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
					if(max == nullptr || comp(max->data[max->filled - 1], iter->data[iter->filled - 1])) {
						max = iter;
					}
				}
			}
			this->top = max->data[max->filled - 1];
			// Test ordering property
			pheet_assert(!comp(ret, this->top));
			parent->reorder_index(this->parent_index);
		}

		return ret;
	}


	void push(T item) {
		pheet_assert(work->dead == 0);

		++filled;
		if(work->filled == 0) {
			work->data[0] = item;
			work->filled = 1;
			if(max == nullptr) {
				max = work;
				this->top = item;
				parent->push(this);
			}
			else {
				pheet_assert(max != work);
				pheet_assert(max->filled > 0);
				if(comp(max->data[max->filled - 1], item)) {
					max = work;
					this->top = item;
					parent->reorder_index(this->parent_index);
				}
			}
		}
		else {
			if(work->filled == 1) {
				if(comp(item, work->data[0])) {
					work->data[1] = work->data[0];
					work->data[0] = item;
				}
				else {
					work->data[1] = item;
					if(max == work) {
						this->top = item;
					}
					else if(comp(max->data[max->filled - 1], item)) {
						max = work;
						this->top = item;
						parent->reorder_index(this->parent_index);
					}
				}
				work->filled = 2;
				if(work->size == 2) {
					put_work();
				}
			}
			else {
				if(comp(item, work->data[work->filled - 1])) {
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
					if(max == work) {
						this->top = item;
					}
					else if(comp(max->data[max->filled - 1], item)) {
						max = work;
						this->top = item;
						parent->reorder_index(this->parent_index);
					}
					if(work->filled == work->size) {
						// We have completely filled up our work array. Put it into the data-structure and retrieve new work array
						put_work();
					}
				}
			}
		}
	}

private:
	SA* merge(SA* item) {
		pheet_assert(item->filled > 0);
		pheet_assert(item->next != nullptr);
		pheet_assert(item->next->filled > 0);
		if((item->next->filled + item->next->dead) == 0) {
			item->next = item->next->next;
			return item;
		}
		size_t total_size = item->filled + item->next->filled;
		size_t total_dead = item->dead + item->next->dead;
		SA* result = find_slot(total_size + total_dead);
		pheet_assert(result->size >= total_size + total_dead);
		result->next = item->next->next;

		size_t dead_offset = total_size;
		for(size_t i = 0; i < item->dead; ++i, ++dead_offset) {
			result->data[dead_offset] = item->data[item->filled + i];
		}
		for(size_t i = 0; i < item->next->dead; ++i, ++dead_offset) {
			result->data[dead_offset] = item->next->data[item->next->filled + i];
		}
		pheet_assert(dead_offset == total_size + total_dead);

		size_t a = 0, b = 0, c = 0;
		while(a < item->filled - 1 && !sr.template is_active<Strategy>(item->data[a])) {
			--total_size;
			++total_dead;
			result->data[total_size] = item->data[a];
			++a;
		}
		while(b < item->next->filled - 1 && !sr.template is_active<Strategy>(item->next->data[b])) {
			--total_size;
			++total_dead;
			result->data[total_size] = item->next->data[b];
			++b;
		}
		while(a < item->filled && b < item->next->filled) {
			if(comp(item->data[a], item->next->data[b])) {
				result->data[c] = item->data[a];
				++a;
				while(a < item->filled - 1 && !sr.template is_active<Strategy>(item->data[a])) {
					--total_size;
					++total_dead;
					result->data[total_size] = item->data[a];
					++a;
				}
			}
			else {
				result->data[c] = item->next->data[b];
				++b;
				while(b < item->next->filled - 1 && !sr.template is_active<Strategy>(item->next->data[b])) {
					--total_size;
					++total_dead;
					result->data[total_size] = item->next->data[b];
					++b;
				}
			}
			++c;
		}
		if(a < item->filled) {
			while(a < (item->filled - 1)) {
				if(sr.template is_active<Strategy>(item->data[a])) {
					result->data[c] = item->data[a];
					++a;
					++c;
				}
				else {
					--total_size;
					++total_dead;
					result->data[total_size] = item->data[a];
					++a;
				}
			}
			result->data[c] = item->data[a];
			++a;
			++c;
		}
		else if(b < item->next->filled) {
			while(b < (item->next->filled - 1)) {
				if(sr.template is_active<Strategy>(item->next->data[b])) {
					result->data[c] = item->next->data[b];
					++b;
					++c;
				}
				else {
					--total_size;
					++total_dead;
					result->data[total_size] = item->next->data[b];
					++b;
				}
			}
			result->data[c] = item->next->data[b];
			++b;
			++c;
		}
		if(max == item || max == item->next) {
			max = result;
			this->top = result->data[total_size - 1];
		}
		item->filled = 0;
		item->dead = 0;
		item->next->filled = 0;
		item->next->dead = 0;
		result->filled = total_size;
		result->dead = total_dead;
		return result;
	}

	void put_work() {
		while(work->next != nullptr && (work->next->filled + work->next->dead) <= (work->filled + work->dead)) {
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
		pheet_assert(arrays[slot].dead == 0);
		pheet_assert(arrays[slot].size >= length);
		return arrays + slot;
	}

	MergeStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	StrategyRetriever& sr;

	size_t filled;
	SA* last_initialized_array;
//	size_t num_filled_slots;
	SA* work;
	SA* max;

//	SA* slots[63];
//	SA* filled_slots[63];
	SA arrays[126];

	MergeStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>* parent;
};

} /* namespace pheet */
#endif /* MERGESTRATEGYITEMHEAP_H_ */
