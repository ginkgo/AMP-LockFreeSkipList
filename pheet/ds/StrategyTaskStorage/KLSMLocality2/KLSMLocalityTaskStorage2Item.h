/*
 * KLSMLocalityTaskStorage2Item.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE2ITEM_H_
#define KLSMLOCALITYTASKSTORAGE2ITEM_H_

#include <atomic>

namespace pheet {

template <class Pheet, class Place, class BaseItem, class Strategy>
struct KLSMLocalityTaskStorage2Item : public BaseItem {
	typedef typename BaseItem::T T;

	KLSMLocalityTaskStorage2Item()
	:owner(nullptr), used_locally(false) {
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
	}

	Place* owner;
	Strategy strategy;
	bool used_locally;
};

template <class Item>
struct KLSMLocalityTaskStorage2ItemReuseCheck {
	bool operator() (Item& item) {
	/*	if(!item.used_locally) { // No need to check for taken items (dead items will never be taken, in fact)
			pheet_assert(item.frame.load(std::memory_order_relaxed) != nullptr);
			pheet_assert(item.taken.load(std::memory_order_relaxed));
			pheet_assert(item.phase != -1);

			return item.frame.load(std::memory_order_relaxed)->can_reuse(item.phase);
		}*/
		return false;
	}
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE2ITEM_H_ */
