/*
 * BasicLinkedListStrategyTaskStorageDataBlock.h
 *
 *  Created on: 07.04.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICLINKEDLISTSTRATEGYTASKSTORAGEDATABLOCK_H_
#define BASICLINKEDLISTSTRATEGYTASKSTORAGEDATABLOCK_H_

#include <pheet/misc/atomics.h>


namespace pheet {

/*
 * less for age. (younger is less)
 */
/*template <class DataBlock>
struct BasicLinkedListStrategyTaskStorageDataBlockAgeComparator {
	bool operator()(DataBlock* b1, DataBlock* b2) {
		return ((ptrdiff_t)(b1->get_first_view_id() - b2->get_first_view_id())) > 0;
	}
};*/

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
class BasicLinkedListStrategyTaskStorageDataBlock {
public:
	typedef TT T;
	typedef typename T::Item Item;
	typedef BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize> Self;
//	typedef BasicLinkedListStrategyTaskStorageDataBlockAgeComparator<Self> AgeComparator;

	BasicLinkedListStrategyTaskStorageDataBlock();
	~BasicLinkedListStrategyTaskStorageDataBlock();

//	void delete_predecessors();

	bool local_take(size_t index, typename T::Item& ret, TaskStorage* ts);
	bool take(size_t index, size_t stored_taken_offset, typename T::Item& ret);
	bool take(size_t index, size_t stored_taken_offset);
	void mark_removed(size_t index, TaskStorage* ts);
	T& peek(size_t index);

	size_t push(T&& item, TaskStorage* ts);

	inline T& get_data(size_t index) {
		pheet_assert(index < BlockSize);
		return data[index];
	}
	inline T const& get_data(size_t index) const {
		pheet_assert(index < BlockSize);
		return data[index];
	}
	inline typename Pheet::Scheduler::BaseStrategy* get_strategy(size_t index, size_t stored_taken_offset) {
		if(data[index].taken != stored_taken_offset) {
			return nullptr;
		}
		return data[index].strategy;
	}
	inline Self* const& get_next() { return next; }
	inline bool is_active() {
		pheet_assert(active != 0 || next != nullptr);
		return active != 0;
	}
	inline bool is_taken(size_t index) {
		return data[index].taken != taken_offset;
	}
	inline bool is_taken(size_t index, size_t stored_taken_offset) {
		return data[index].taken != stored_taken_offset;
	}
	inline bool is_first() {
		return prev == nullptr;
	}
	inline size_t get_size() { return filled; }
	inline size_t get_max_size() { return BlockSize; }

	inline size_t get_taken_offset() { return taken_offset; }

	void reset_content();
	void reuse(size_t id, Self* prev);

	bool is_reusable() {
		return reg == 0 && num_pred == 0;
	}

	void acquire_block() {
		SIZET_ATOMIC_ADD(&reg, 1);
	}
	void deregister() {
		pheet_assert(reg > 0);
		SIZET_ATOMIC_SUB(&reg, 1);
	}

	void add_predecessor() {
		++num_pred;
	}
	void sub_predecessor() {
		--num_pred;
	}

private:
	void clean(TaskStorage* ts);

	T data[BlockSize];

	size_t id;
	Self* prev;
	Self* next;
	size_t num_pred;
	size_t reg;

//	Self* orig_prev;
//	Self* orig_next;
//	Self* next_freed;
	size_t filled;

	size_t active;

	size_t taken_offset;
};

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::BasicLinkedListStrategyTaskStorageDataBlock()
:id(0), prev(nullptr), next(nullptr), num_pred(0), reg(0), filled(0), active(BlockSize), taken_offset(0) {
/*	if(orig_prev != nullptr) {
		pheet_assert(orig_prev->orig_next == nullptr);
		orig_prev->orig_next = this;
	}*/
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::~BasicLinkedListStrategyTaskStorageDataBlock() {
	// Delete remaining strategies
	for(size_t i = 0; i < filled; ++i) {
		pheet_assert(data[i].taken == taken_offset + 1);
		delete data[i].strategy;
	}
}
/*
template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::delete_predecessors() {
	Self* p1 = prev;
	while(p1 != nullptr) {
		Self* p2 = p1->prev;
		delete p1;
		p1 = p2;
	}
}*/

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::mark_removed(size_t index, TaskStorage* ts) {
	pheet_assert(active > 0);
	pheet_assert((data[index].taken & 1) == 1);

	--active;
	clean(ts);
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
bool BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::local_take(size_t index, typename T::Item& ret, TaskStorage* ts) {
	pheet_assert(active > 0);
	pheet_assert(index < filled);
	pheet_assert((taken_offset & 1) == 0);
	--active;

	if(data[index].taken == taken_offset) {
		if(SIZET_CAS(&(data[index].taken), taken_offset, taken_offset + 1)) {
			ret = data[index].item;
			clean(ts);
			return true;
		}
	}
	clean(ts);
	return false;
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
bool BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::take(size_t index, size_t stored_taken_offset, typename T::Item& ret) {
	pheet_assert((stored_taken_offset & 1) == 0);

	if(data[index].taken == stored_taken_offset) {
		pheet_assert(index < filled);
		if(SIZET_CAS(&(data[index].taken), stored_taken_offset, stored_taken_offset + 1)) {
			ret = data[index].item;
			return true;
		}
	}
	return false;
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
bool BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::take(size_t index, size_t stored_taken_offset) {
//	pheet_assert(index < filled);
	pheet_assert((stored_taken_offset & 1) == 0);

	if(data[index].taken == stored_taken_offset) {
		if(SIZET_CAS(&(data[index].taken), stored_taken_offset, stored_taken_offset + 1)) {
			return true;
		}
	}
	return false;
}
/*
template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::mark_removed(size_t index, TaskStorage* ts) {
	pheet_assert(active > 0);
	pheet_assert(index < filled);
	--active;
	clean(ts);
}*/

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
TT& BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::peek(size_t index) {
	pheet_assert(active > 0);
	pheet_assert(index < filled);

	return data[index].item;
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::clean(TaskStorage* ts) {
	// There should be no problems with id wraparound, except that block with id 0 ignores rules
	// This means O(log(n)) is not satisfied in for every predecessor of block 0
	// This leads to a complexity of O((n/(size_t_max + 1))*log(size_t_max)) for n > size_t max which isn't really bad
	// and rare in any case (it's linear to the number of wraparounds and not logarithmic)

	pheet_assert(active != 0 || next != nullptr);
	if(active == 0 // Check if block is not used any more
			&& (num_pred == 0 // Either no predecessor exists anymore
					// Or make sure the next block is a higher power of two (to guarantee O(log(n)) access times
					// for other threads missing elements (don't want them to have O(n) for elements they never need)
					|| ((next->id & id) != id))) {
		 // Only free blocks if they have a successor (I believe that it may only be inactive if there is a successor)
		pheet_assert(next != nullptr);
		if(next->prev == this)
			next->prev = prev;
		if(prev != nullptr) {
			prev->next = next;
			pheet_assert(num_pred > 0);
			--num_pred;
			++(next->num_pred);
			// Tail recursion
			prev->clean(ts);
		}
	}
}

template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
size_t BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::push(T&& item, TaskStorage* ts) {
	pheet_assert(filled < BlockSize);

	data[filled] = item;

	if(filled == BlockSize - 1) {
		Self* tmp = ts->create_block();
		tmp->reuse(id + 1, this);
		MEMORY_FENCE();
		next = tmp;
	}
	else {
		MEMORY_FENCE();
	}
	++filled;
	return filled - 1;
}


template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::reset_content() {
	// TODO: can we do this earlier? when we notice block is not active any more or something? maybe even delete strategies after each take?
	pheet_assert(filled == BlockSize);
	for(size_t i = 0; i < filled; ++i) {
		pheet_assert(data[i].taken == taken_offset + 1);
		++(data[i].taken);
		delete data[i].strategy;
	}
	filled = 0;
}


template <class Pheet, typename TT, class TaskStorage, size_t BlockSize>
void BasicLinkedListStrategyTaskStorageDataBlock<Pheet, TT, TaskStorage, BlockSize>::reuse(size_t id, Self* prev) {
	if(filled == BlockSize) {
		reset_content();
	}
	if(next != nullptr) {
		pheet_assert(next->num_pred > 0);
		--(next->num_pred);
		next->prev = nullptr;
	}
	pheet_assert(filled == 0);

	this->id = id;
	this->prev = prev;
	next = nullptr;
	active = BlockSize;
	taken_offset += 2;

	pheet_assert(num_pred == 0);
	if(prev != nullptr) {
		num_pred = 1;
	}
}



}

#endif /* BASICLINKEDLISTSTRATEGYTASKSTORAGEDATABLOCK_H_ */
