/*
 * SortingTests.cpp
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "SortingTests.h"

#ifdef SORTING_TEST
#include "Reference/ReferenceSTLSort.h"
#include "Reference/ReferenceQuicksort.h"
#include "Reference/ReferenceQuicksortLoop.h"
#include "Strategy/StrategyQuicksort.h"
#include "Strategy2/Strategy2Quicksort.h"
#include "Dag/DagQuicksort.h"
#include "MixedMode/MixedModeQuicksort.h"
#include "Reference/ReferenceHeapSort.h"

//#include <pheet/ds/StealingDeque/CircularArray11/CircularArrayStealingDeque11.h>
#include <pheet/ds/PriorityQueue/Heap/Heap.h>
#include <pheet/ds/PriorityQueue/Merge/MergeHeap.h>
#include <pheet/ds/PriorityQueue/STLPriorityQueueWrapper/STLPriorityQueueWrapper.h>
//#include <pheet/ds/PriorityQueue/SortedArrayHeap/SortedArrayHeap.h>
#include <pheet/ds/PriorityQueue/Fibonacci/FibonacciHeap.h>
#include <pheet/ds/PriorityQueue/FibonacciSame/FibonacciSameHeap.h>
#include <pheet/ds/PriorityQueue/Fibolike/FibolikeHeap.h>
#include <pheet/ds/Queue/GlobalLock/GlobalLockQueue.h>
#include <pheet/ds/MultiSet/GlobalLock/GlobalLockMultiSet.h>
#include <pheet/ds/StrategyHeap/Volatile/VolatileStrategyHeap.h>
#include <pheet/ds/StrategyTaskStorage/CentralK/CentralKStrategyTaskStorage.h>
#include <pheet/ds/StrategyTaskStorage/DistK/DistKStrategyTaskStorage.h>

#include <pheet/primitives/Mutex/TASLock/TASLock.h>
#include <pheet/primitives/Mutex/TTASLock/TTASLock.h>

#include <pheet/pheet.h>
#include <pheet/sched/Basic/BasicScheduler.h>
#include <pheet/sched/Finisher/FinisherScheduler.h>
#include <pheet/sched/Centralized/CentralizedScheduler.h>
#include <pheet/sched/CentralizedPriority/CentralizedPriorityScheduler.h>
#include <pheet/sched/Strategy/StrategyScheduler.h>
#include <pheet/sched/BStrategy/BStrategyScheduler.h>
#include <pheet/sched/Strategy2/StrategyScheduler2.h>
#include <pheet/sched/Synchroneous/SynchroneousScheduler.h>
#include <pheet/sched/MixedMode/MixedModeScheduler.h>

#include <iostream>
#endif

namespace pheet {


SortingTests::SortingTests() {

}

SortingTests::~SortingTests() {

}

void SortingTests::run_test() {
#ifdef SORTING_TEST
	std::cout << "----" << std::endl;

#ifdef AMP_STEALING_DEQUE_TEST
//	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>::WithTaskStorage<YourImplementation>,
//						DagQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>,
						DagQuicksortNoCut>();

#elif AMP_QUEUE_STACK_TEST

	this->run_sorter<	Pheet::WithScheduler<CentralizedScheduler>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<CentralizedScheduler>::WithTaskStorage<GlobalLockQueue>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet,
						DagQuicksortNoCut>();
#elif AMP_HEAP_TEST
	this->run_sorter<	Pheet::WithScheduler<CentralizedPriorityScheduler>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<CentralizedPriorityScheduler>::WithPriorityTaskStorage<GlobalLockMultiSetPriorityQueue>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>,
						DagQuicksortNoCut>();

#elif AMP_LOCK_TEST

	this->run_sorter<	Pheet::WithScheduler<CentralizedScheduler>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<CentralizedScheduler>::WithMutex<TASLock>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<CentralizedScheduler>::WithMutex<TTASLock>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>,
						DagQuicksortNoCut>();
	this->run_sorter<	Pheet,
						DagQuicksortNoCut>();
#else
	// default tests
	this->run_sorter<	Pheet::WithScheduler<StrategyScheduler2>,
						Strategy2Quicksort>();
/*	this->run_sorter<	Pheet::WithScheduler<StrategyScheduler2>,
						DagQuicksort>();

	this->run_sorter<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<DistKStrategyTaskStorage>,
						StrategyQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<DistKStrategyTaskStorageLocalK>,
						StrategyQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<CentralKStrategyTaskStorage>,
						StrategyQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<CentralKStrategyTaskStorageLocalK>,
						StrategyQuicksort>();

	this->run_sorter<	Pheet::WithScheduler<StrategyScheduler>,
						StrategyQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<StrategyScheduler>,
						DagQuicksort>();*/

	this->run_sorter<	Pheet::WithScheduler<MixedModeScheduler>,
						MixedModeQuicksort>();

	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>,
						DagQuicksort>();
//	this->run_sorter<	Pheet::WithScheduler<BasicScheduler>::WithStealingDeque<CircularArrayStealingDeque11>,
//						DagQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						DagQuicksort>();
	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						ReferenceSTLSort>();
	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						ReferenceQuicksort>();
/*	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						ReferenceQuicksortLoop>();*/
//	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
//						ReferenceHeapSort<Pheet>::WithPriorityQueue<FibonacciSameHeap>::template BT >();
//	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
//						ReferenceHeapSort<Pheet>::WithPriorityQueue<FibolikeHeap>::template BT >();
//	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
//						ReferenceHeapSort<Pheet>::WithPriorityQueue<FibonacciHeap>::template BT >();
/*	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						ReferenceHeapSort<Pheet>::WithPriorityQueue<MergeHeap>::template BT >();
	this->run_sorter<	Pheet::WithScheduler<SynchroneousScheduler>,
						ReferenceHeapSort>();*/
//	this->run_sorter<	Pheet::WithScheduler<FinisherScheduler>,
//						DagQuicksort>();
#endif

#endif
}


}
