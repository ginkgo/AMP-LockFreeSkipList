/*
 * CentralKStrategyTaskStorage.h
 *
 *  Created on: 03.10.2013
 *      Author: mwimmer, mpoeter
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGE_CPP11_H_
#define CENTRALKSTRATEGYTASKSTORAGE_CPP11_H_

#include "../../StrategyHeap/Basic/BasicStrategyHeap.h"
#include "../../StrategyHeap/Merge/MergeStrategyHeap.h"

#include "CentralKStrategyTaskStoragePlace.h"
#include "CentralKStrategyTaskStorageDataBlock.h"

namespace pheet { namespace cpp11 {

template <class Pheet, typename TT, template <class SP, typename ST, class SR> class StrategyHeapT, size_t BlockSize, size_t Tests, bool LocalKPrio>
class CentralKStrategyTaskStorageImpl {
public:
	typedef CentralKStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT, BlockSize, Tests, LocalKPrio> Self;

	typedef CentralKStrategyTaskStoragePlace<Pheet, Self, TT, StrategyHeapT, BlockSize, Tests, LocalKPrio> Place;
	typedef CentralKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize, Tests> DataBlock;

	typedef TT T;

	template <template <class SP, typename ST, class SR> class NewHeap>
	using WithStrategyHeap = CentralKStrategyTaskStorageImpl<Pheet, TT, NewHeap, BlockSize, Tests, LocalKPrio>;

	template <size_t NewBlockSize>
	using WithBlockSize = CentralKStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT, NewBlockSize, Tests, LocalKPrio>;

	template <size_t NewTests>
	using WithTests = CentralKStrategyTaskStorageImpl<Pheet, TT, StrategyHeapT, BlockSize, NewTests, LocalKPrio>;

	CentralKStrategyTaskStorageImpl(size_t num_places)
	:tail(0), start_block(nullptr), num_places(num_places) {}
	~CentralKStrategyTaskStorageImpl() {}

	size_t get_num_places() const {
		return num_places;
	}

	static void print_name() {
		std::cout << "cpp11::CentralKStrategyTaskStorage<" << BlockSize << ", " << Tests << ", " << (LocalKPrio?"Local":"Global") << ">";
	}

	std::atomic<size_t> tail;

	DataBlock* start_block;
private:
	const size_t num_places;
};

template <class Pheet, typename TT>
using CentralKStrategyTaskStorage = CentralKStrategyTaskStorageImpl<Pheet, TT, BasicStrategyHeap, 512, 128, false>;
template <class Pheet, typename TT>
using CentralKStrategyTaskStorageMergeHeap = CentralKStrategyTaskStorageImpl<Pheet, TT, MergeStrategyHeap, 512, 128, false>;

template <class Pheet, typename TT>
using CentralKStrategyTaskStorageLocalK = CentralKStrategyTaskStorageImpl<Pheet, TT, BasicStrategyHeap, 512, 128, true>;

} // namespace cpp11
} // namespace pheet

#endif /* CENTRALKSTRATEGYTASKSTORAGE_CPP11_H_ */
