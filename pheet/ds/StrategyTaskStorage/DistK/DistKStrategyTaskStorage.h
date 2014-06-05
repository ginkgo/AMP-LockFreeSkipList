/*
 * DistKStrategyTaskStorage.h
 *
 *  Created on: 15.01.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DISTKSTRATEGYTASKSTORAGE_H_
#define DISTKSTRATEGYTASKSTORAGE_H_

#include "DistKStrategyTaskStoragePlace.h"
#include "DistKStrategyTaskStorageDataBlock.h"
#include <pheet/ds/StrategyHeap/Basic/BasicStrategyHeap.h>

namespace pheet {



template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize, bool LocalKPrio>
class DistKStrategyTaskStorageImpl {
public:
	typedef DistKStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT, BlockSize, LocalKPrio> Self;

	typedef DistKStrategyTaskStoragePlace<Pheet, Self, TT, StrategyHeapT, BlockSize, LocalKPrio> Place;
	typedef DistKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize> DataBlock;

	typedef TT T;

	template <template <class SP, typename ST, class SR> class NewHeap>
	using WithStrategyHeap = DistKStrategyTaskStorageImpl<Pheet, TT, NewHeap, BlockSize, LocalKPrio>;

	template <size_t NewBlockSize>
	using WithBlockSize = DistKStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT, NewBlockSize, LocalKPrio>;

	DistKStrategyTaskStorageImpl(size_t num_places)
	:start_block(nullptr), num_places(num_places) {}
	~DistKStrategyTaskStorageImpl() {}

	size_t get_num_places() {
		return num_places;
	}

	DataBlock* start_block;

	static void print_name() {
		std::cout << "DistKStrategyTaskStorage<" << BlockSize << ", " << (LocalKPrio?"Local":"Global") << ">";
	}

private:
	size_t num_places;
};

template <class Pheet, typename TT>
using DistKStrategyTaskStorage = DistKStrategyTaskStorageImpl<Pheet, TT, BasicStrategyHeap, 128, false>;

template <class Pheet, typename TT>
using DistKStrategyTaskStorageLocalK = DistKStrategyTaskStorageImpl<Pheet, TT, BasicStrategyHeap, 128, true>;

} /* namespace pheet */
#endif /* DISTKSTRATEGYTASKSTORAGE_H_ */
