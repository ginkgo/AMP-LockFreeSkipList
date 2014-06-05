/*
 * DistKStrategyTaskStorageItem.h
 *
 *  Created on: 15.01.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DISTKSTRATEGYTASKSTORAGEITEM_H_
#define DISTKSTRATEGYTASKSTORAGEITEM_H_

namespace pheet {

template <class Pheet, class Place, typename TT>
struct DistKStrategyTaskStorageItem {
	typedef DistKStrategyTaskStorageItem<Pheet, Place, TT> Self;
	typedef TT Data;

	typename Pheet::Scheduler::BaseStrategy* strategy;
	TT data;
	size_t position;
	size_t orig_position;

	typename Place::DataBlock* block;

	void (Place::*item_push)(Self* item, size_t position, uint8_t type);
};

template <class Item>
struct DistKStrategyTaskStorageItemReuseCheck {
	bool operator() (Item const& item) const {
		return item.position != item.orig_position && item.strategy == nullptr;
	}
};

} /* namespace pheet */
#endif /* DISTKSTRATEGYTASKSTORAGEITEM_H_ */
