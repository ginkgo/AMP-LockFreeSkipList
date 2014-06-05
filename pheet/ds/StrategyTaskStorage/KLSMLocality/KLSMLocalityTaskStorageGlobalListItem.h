/*
 * KLSMLocalityTaskStorageGlobalListItem.h
 *
 *  Created on: Oct 9, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGEGLOBALLISTITEM_H_
#define KLSMLOCALITYTASKSTORAGEGLOBALLISTITEM_H_

#include <atomic>

namespace pheet {

template <class Pheet, class Block>
class KLSMLocalityTaskStorageGlobalListItem {
public:
	typedef KLSMLocalityTaskStorageGlobalListItem<Pheet, Block> Self;
	KLSMLocalityTaskStorageGlobalListItem()
	:block(nullptr), registered(0), next(this), id(0), next_id(1) {

	}
	~KLSMLocalityTaskStorageGlobalListItem() {

	}

	void reset() {
//		pheet_assert(is_reusable());
//		block.store(nullptr, std::memory_order_relaxed);
		// The thread linking the new item does not decrement registered, so we only need P-1
		registered.store(Pheet::get_num_places() - 1, std::memory_order_relaxed);
		next.store(nullptr, std::memory_order_relaxed);
	}

	/*
	 * Unsafe. Only use if not yet connected to global list
	 */
	Self* get_next() {
		return next.load(std::memory_order_relaxed);
	}

	Self* move_next() {
		Self* ret = next.load(std::memory_order_relaxed);
		if(ret != nullptr) {
			pheet_assert(registered.load(std::memory_order_relaxed) != 0);
			registered.fetch_sub(1, std::memory_order_seq_cst);

			// Reread next pointer, essential for skipping of items
			// For this to work, updates to registered need to be sequentially consistent with
			// updates to the next pointer
			ret = next.load(std::memory_order_relaxed);
			// Skipping should never happen if no successor exists
			// This item cannot be reused as long as we have not decremented the ref counter of the next item
			// as well
			pheet_assert(ret != nullptr);
		}
		return ret;
	}

	Block* get_block() {
		return block.load(std::memory_order_relaxed);
	}

	Block* acquire_block() {
		return block.load(std::memory_order_acquire);
	}

	void mark_reusable() {
	//	registered.store(0, std::memory_order_relaxed);
		next_id.store(id.load(std::memory_order_relaxed), std::memory_order_relaxed);
	}

	bool is_reusable() {
		// Check if we have a successor - next will only be changed locally so we can safely assume it stays
		// the same throughout the function call
		Self* n = next.load(std::memory_order_relaxed);
		if(n == nullptr) {
			pheet_assert(registered.load(std::memory_order_relaxed) != 0 || Pheet::get_num_places() == 1);
			return false;
		}

		// Check whether successor has been freed
		size_t reg = n->registered.load(std::memory_order_acquire);
		if(reg == 0) {
			// Great, we can safely reuse this item
			return true;
		}

		// Expected id of successors successor needs to be read before ABA check
		size_t nnext_id = n->next_id.load(std::memory_order_acquire);
		// Find successors successor (also needs to be read before ABA check)
		Self* nn = n->next.load(std::memory_order_acquire);

		size_t nid = next_id.load(std::memory_order_relaxed);
		// ABA check in case next item has already been reused
		if(nid != n->id.load(std::memory_order_relaxed)) {
			// Next item has been reused, so we are safe to reuse this item
			return true;
		}

		// Skipping only makes sense if block is empty
		if(n->get_block() != nullptr) {
			return false;
		}

		// Power of 2 trick - Only skip item if we are allowed to skip next item, but successor is not
		// Can shrink a linked list to a size log of its length, but needs strictly increasing numbering
		// to work
		if((get_id() | nid) != nid || (nid | nnext_id) == nnext_id) {
			return false;
		}

		if(nn == nullptr) {
			// No way to skip
			return false;
		}

		// ABA check for successors successor
		if(nnext_id != nn->id.load(std::memory_order_relaxed)) {
			// Skipping is dangerous right now, next item might be in the process of skipping an item
			return false;
		}

		// We can safely skip next item now. First store the new id for the next item
		next_id.store(nnext_id, std::memory_order_relaxed);

		// Now update next pointer. Needs to be sequentially consistent to be correctly
		// ordered with regards to updates to registered
		next.store(nn, std::memory_order_seq_cst);

		// Now see if no thread has a reference to our old next item (seq_cst on next.store is essential for this
		// to work)
		if(registered.load(std::memory_order_relaxed) == reg && n->registered.load(std::memory_order_relaxed) == reg) {
			// No references possible, mark n as reusable!
			n->mark_reusable();
		}

		// Otherwise we have to wait for new next item to be freed

		return false;
	}

	void update_block(Block* b) {
		block.store(b, std::memory_order_release);
	}

	bool link(Self* i) {
		Self* n = next.load(std::memory_order_relaxed);
		if(n == nullptr) {
			pheet_assert(i->registered.load(std::memory_order_relaxed) == Pheet::get_num_places() - 1);
			pheet_assert(i->id.load(std::memory_order_relaxed) == id.load(std::memory_order_relaxed) + 1);
			pheet_assert(next_id.load(std::memory_order_relaxed) == id.load(std::memory_order_relaxed) + 1);
			if(next.compare_exchange_strong(n, i, std::memory_order_release, std::memory_order_acquire)) {
				return true;
			}
		}
		return false;
	}

	void local_link(Self* i) {
//		pheet_assert(next.load(std::memory_order_relaxed) == nullptr);
		pheet_assert(i == nullptr || i->registered.load(std::memory_order_relaxed) == Pheet::get_num_places() - 1);
		next.store(i, std::memory_order_relaxed);
	}

	size_t get_id() {
		return id.load(std::memory_order_relaxed);
	}

	void set_id(size_t value) {
		id.store(value, std::memory_order_relaxed);
		next_id.store(value + 1, std::memory_order_relaxed);
	}

private:
	std::atomic<Block*> block;
	std::atomic<procs_t> registered;
	std::atomic<Self*> next;
	std::atomic<size_t> id;
	std::atomic<size_t> next_id;
};


template <class Item>
struct KLSMLocalityTaskStorageGlobalListItemReuseCheck {
	bool operator() (Item& item) {
		return item.is_reusable();
	}
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGEGLOBALLISTITEM_H_ */
