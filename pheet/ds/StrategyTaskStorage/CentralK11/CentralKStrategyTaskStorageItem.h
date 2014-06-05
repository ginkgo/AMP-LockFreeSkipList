/*
 * CentralKStrategyTaskStorageItem.h
 *
 *  Created on: 03.10.2013
 *      Author: mwimmer, mpoeter
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEITEM_CPP11_H_
#define CENTRALKSTRATEGYTASKSTORAGEITEM_CPP11_H_

namespace pheet { namespace cpp11 {

template <class Pheet, class Place, typename TT>
struct CentralKStrategyTaskStorageItem {
	typedef CentralKStrategyTaskStorageItem<Pheet, Place, TT> Self;
	typedef TT Data;

	CentralKStrategyTaskStorageItem()
		:strategy(nullptr), position(1), orig_position(0) {

	}

	~CentralKStrategyTaskStorageItem() {
		// Some items may not be cleaned up at the end, and cannot be cleaned up at data-block level
		// due to race conditions. Therefore delete at the item level.
		delete strategy;
		strategy = nullptr;
	}


	typename Pheet::Scheduler::BaseStrategy* strategy;
	TT data;
	std::atomic<size_t> position;
	size_t orig_position;

	Place* owner;

	void (Place::*item_push)(Self* item, size_t position);
};

template <class Item>
struct CentralKStrategyTaskStorageItemReuseCheck {
	bool operator() (Item const& item) const {
		return item.position.load(std::memory_order_relaxed) != item.orig_position && item.strategy == nullptr;
	}
};

} // namespace cpp11
} // namespace pheet

#endif /* CENTRALKSTRATEGYTASKSTORAGEITEM_CPP11_H_ */
