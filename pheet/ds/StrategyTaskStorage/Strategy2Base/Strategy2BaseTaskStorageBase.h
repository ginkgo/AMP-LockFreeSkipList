/*
 * StrategyScheduler2TaskStorageBase.h
 *
 *  Created on: Aug 12, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2BASETASKSTORAGEBASE_H_
#define STRATEGY2BASETASKSTORAGEBASE_H_

#include "Strategy2BaseTaskStorageItem.h"

namespace pheet {

template <class Pheet, typename TT>
class Strategy2BaseTaskStorageBase {
public:
	typedef Strategy2BaseTaskStorageBase<Pheet, TT> Self;
	typedef Strategy2BaseTaskStorageBaseItem<Pheet, Self, TT> BaseItem;

	Strategy2BaseTaskStorageBase() {}
	virtual ~Strategy2BaseTaskStorageBase() {}

	/**
	 * Needs to contain at least one release statement or fence on success
	 * to make sure the parent task storage is correct. Release has to occur at latest at
	 * linearization point of success
	 * No requirements for failure
	 */
	virtual TT pop(BaseItem* boundary, procs_t place_id) = 0;
	virtual TT steal(BaseItem* boundary, procs_t place_id) = 0;
};

template <class Pheet>
class Strategy2BaseTaskStorageBasePlace {
public:
	typedef Strategy2BaseTaskStorageBasePlace<Pheet> Self;

	Strategy2BaseTaskStorageBasePlace() {}
	virtual ~Strategy2BaseTaskStorageBasePlace() {}

	/**
	 * Needs to contain at least one release statement or fence on success
	 * to make sure the parent task storage is correct. Release has to occur at latest at
	 * linearization point of success
	 * No requirements for failure
	 */
	virtual void clean_up() = 0;
};

} /* namespace pheet */
#endif /* STRATEGY2BASETASKSTORAGEBASE_H_ */
