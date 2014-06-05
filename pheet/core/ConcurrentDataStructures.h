/*
 * ConcurrentDataStructures.h
 *
 *  Created on: 12.02.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CONCURRENTDATASTRUCTURES_H_
#define CONCURRENTDATASTRUCTURES_H_

#include "../ds/CircularArray/FixedSize/FixedSizeCircularArray.h"
#include "../ds/CircularArray/TwoLevelGrowing/TwoLevelGrowingCircularArray.h"
#include "../ds/StealingDeque/CircularArray/CircularArrayStealingDeque.h"
#include "../ds/Stack/GlobalLock/GlobalLockStack.h"
#include "../ds/Queue/GlobalLock/GlobalLockQueue.h"

namespace pheet {

template <class Pheet, template <class, typename> class StackT, template <class, typename> class QueueT, template <class P, typename T> class CircularArrayT, template <class P, typename T> class FixedSizeCircularArrayT, template <class P, typename T> class StealingDequeT>
class ConcurrentDataStructuresEnv {
public:
	template<typename T>
		using Stack = StackT<Pheet, T>;
	template<typename T>
		using Queue = QueueT<Pheet, T>;

	template<typename T>
		using CircularArray = CircularArrayT<Pheet, T>;
	template<typename T>
		using FixedSizeCircularArray = FixedSizeCircularArrayT<Pheet, T>;
	template<typename T>
		using StealingDeque = StealingDequeT<Pheet, T>;

	template<template <class P, typename> class T>
		using WithStealingDeque = ConcurrentDataStructuresEnv<Pheet, StackT, QueueT, CircularArrayT, FixedSizeCircularArrayT, T>;

	template <class P>
		using BT = ConcurrentDataStructuresEnv<P, StackT, QueueT, CircularArrayT, FixedSizeCircularArrayT, StealingDequeT>;
};

template<class Pheet>
using ConcurrentDataStructures = ConcurrentDataStructuresEnv<Pheet, GlobalLockStack, GlobalLockQueue, TwoLevelGrowingCircularArray, FixedSizeCircularArray, CircularArrayStealingDeque>;

}

#endif /* CONCURRENTDATASTRUCTURES_H_ */
