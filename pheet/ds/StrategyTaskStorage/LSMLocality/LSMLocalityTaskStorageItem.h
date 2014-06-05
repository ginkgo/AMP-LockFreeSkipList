/*
 * LSMLocalityTaskStorageItem.h
 *
 *  Created on: Sep 18, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LSMLOCALITYTASKSTORAGEITEM_H_
#define LSMLOCALITYTASKSTORAGEITEM_H_

#include <atomic>

namespace pheet {

template <class Pheet, class Place, class Frame, class BaseItem, class Strategy>
struct LSMLocalityTaskStorageItem : public BaseItem {
	typedef typename BaseItem::T T;

	LSMLocalityTaskStorageItem()
	:owner(nullptr), used_locally(false), frame(nullptr), phase(0) {
	}

	/*
	 * Is only safe to be called if the thread owns the item or is registered to the frame
	 */
	bool is_taken() {
		return this->taken.load(std::memory_order_acquire);
	}

	/*
	 * Is only safe to be called if the thread owns the item or is registered to the frame
	 */
	bool is_taken_or_dead() {
		return this->taken.load(std::memory_order_acquire) || strategy.dead_task();
	}

	/*
	 * Is only safe to be called if the thread owns the item or is registered to the frame
	 */
	bool is_dead() {
		return strategy.dead_task();
	}

	T take() {
		bool expected = false;
		if(this->taken.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_acquire)) {
			this->version.store(this->version.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
			return this->data;
		}
		return nullable_traits<T>::null_value;
	}

	/*
	 * Local version of take_and_delete, called by owner
	 * Tries to take item and deletes it on success
	 */
	void take_and_delete() {
		bool expected = false;
		if(this->taken.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_acquire)) {
			this->version.store(this->version.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

			this->data.drop_item();
		}
	}

	void free_locally() {
		pheet_assert(used_locally);
		used_locally = false;
		pheet_assert(phase == -1);
		Frame* f = frame.load(std::memory_order_relaxed);
		f->register_reusable();
		phase = (ptrdiff_t) f->get_phase();
	}

	Place* owner;
	Strategy strategy;
	bool used_locally;

	std::atomic<Frame*> frame;
	ptrdiff_t phase;
};

template <class Item, class Frame>
struct LSMLocalityTaskStorageItemReuseCheck {
	bool operator() (Item& item) {
		if(!item.used_locally) { // No need to check for taken items (dead items will never be taken, in fact)
			pheet_assert(item.frame.load(std::memory_order_relaxed) != nullptr);
			pheet_assert(item.taken.load(std::memory_order_relaxed));
			pheet_assert(item.phase != -1);

			return item.frame.load(std::memory_order_relaxed)->can_reuse(item.phase);
		}
		return false;
	}
};

} /* namespace pheet */
#endif /* LSMLOCALITYTASKSTORAGEITEM_H_ */
