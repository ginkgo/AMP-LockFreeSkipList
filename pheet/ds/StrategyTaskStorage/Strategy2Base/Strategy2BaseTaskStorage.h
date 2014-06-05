/*
 * Strategy2BaseTaskStorage.h
 *
 *  Created on: 12.08.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2BASETASKSTORAGE_H_
#define STRATEGY2BASETASKSTORAGE_H_

#include "Strategy2BaseTaskStoragePlace.h"
#include "Strategy2BaseTaskStorageDataBlock.h"
#include "Strategy2BaseTaskStorageBase.h"

namespace pheet {


/**
 * Base task storage with arora-like behaviour (difference is that spying is used instead of stealing)
 * Properties: Only place in strategy is used.
 */
template <class Pheet, typename TT, size_t BlockSize>
class Strategy2BaseTaskStorageImpl : public Strategy2BaseTaskStorageBase<Pheet, TT> {
public:
	typedef Strategy2BaseTaskStorageImpl<Pheet, TT, BlockSize> Self;
	typedef Strategy2BaseTaskStorageBase<Pheet, TT> BaseTaskStorage;

	typedef Strategy2BaseTaskStoragePlace<Pheet, Self, TT, BlockSize> Place;
	typedef Strategy2BaseTaskStorageBasePlace<Pheet> BasePlace;
	typedef Strategy2BaseTaskStorageDataBlock<Pheet, Place, BaseTaskStorage, TT, BlockSize> DataBlock;

	typedef TT T;

	typedef typename BaseTaskStorage::BaseItem BaseItem;

	template <size_t NewBlockSize>
	using WithBlockSize = Strategy2BaseTaskStorageImpl<Pheet, TT, NewBlockSize>;

	Strategy2BaseTaskStorageImpl()
	{}
	~Strategy2BaseTaskStorageImpl() {}

	/**
	 * Overload from parent class
	 * Since this is our root task storage it should never be necessary to call it
	 */
	T pop(BaseItem*, procs_t) {
		// Should never occur. Throw exception if it does.
		throw -1;
	}

	/**
	 * Overload from parent class
	 * Since this is our root task storage it should never be necessary to call it
	 */
	T steal(BaseItem*, procs_t) {
		// Should never occur. Throw exception if it does.
		throw -1;
	}


	static void print_name() {
		std::cout << "Strategy2BaseTaskStorage<" << BlockSize << ">";
	}

private:
};

template <class Pheet, typename TT>
using Strategy2BaseTaskStorage = Strategy2BaseTaskStorageImpl<Pheet, TT, 128>;

} /* namespace pheet */
#endif /* STRATEGY2BASETASKSTORAGE_H_ */
