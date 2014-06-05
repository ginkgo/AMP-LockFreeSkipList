/*
 * GraphBipartitioningTests.cpp
 *
 *  Created on: 07.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "GraphBipartitioningTests.h"

#ifdef GRAPH_BIPARTITIONING_TEST

#include "Basic/BBGraphBipartitioningFREELogic.h"

#include <pheet/models/MachineModel/HWLoc/HWLocSMTMachineModel.h>
#include <pheet/sched/Basic/BasicScheduler.h>
#include <pheet/sched/Centralized/CentralizedScheduler.h>
#include <pheet/sched/Strategy/StrategyScheduler.h>
#include <pheet/sched/Strategy2/StrategyScheduler2.h>
#include <pheet/sched/BStrategy/BStrategyScheduler.h>
#include <pheet/sched/Synchroneous/SynchroneousScheduler.h>

#include <pheet/ds/Queue/GlobalLock/GlobalLockQueue.h>
#include <pheet/ds/MultiSet/GlobalLock/GlobalLockMultiSet.h>
#include <pheet/ds/StrategyTaskStorage/CentralK/CentralKStrategyTaskStorage.h>
#include <pheet/ds/StrategyTaskStorage/CentralK11/CentralKStrategyTaskStorage.h>
#include <pheet/ds/StrategyTaskStorage/DistK/DistKStrategyTaskStorage.h>

#include <pheet/primitives/Mutex/TASLock/TASLock.h>
#include <pheet/primitives/Mutex/TTASLock/TTASLock.h>

#include "Basic/BBGraphBipartitioning.h"
#include "Strategy/StrategyBBGraphBipartitioning.h"
#include "Strategy/StrategyBBGraphBipartitioningBestFirstStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningDepthFirstStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningDepthFirstBestStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningLowerBoundStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningUpperBoundStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningUpperBoundFifoStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningUpperLowerBoundStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningAutoStrategy.h"
#include "Strategy/StrategyBBGraphBipartitioningDynamicStrategy.h"

#include "PPoPP/PPoPPBBGraphBipartitioning.h"
#include "PPoPP/PPoPPBBGraphBipartitioningBestFirstStrategy.h"
#include "PPoPP/PPoPPBBGraphBipartitioningLowerBoundStrategy.h"
#include "PPoPP/PPoPPBBGraphBipartitioningEstimateStrategy.h"
#include "PPoPP/PPoPPBBGraphBipartitioningUpperLowerBoundStrategy.h"



//#include "PPoPP/PPoPPBBGraphBipartitioning.h"


/*
#include "../../sched/strategies/Fifo/FifoStrategy.h"
#include "../../sched/strategies/Lifo/LifoStrategy.h"
#include "../../sched/strategies/LifoFifo/LifoFifoStrategy.h"

#include "../test_schedulers.h"*/
#endif

namespace pheet {

GraphBipartitioningTests::GraphBipartitioningTests() {

}

GraphBipartitioningTests::~GraphBipartitioningTests() {

}

void GraphBipartitioningTests::run_test() {
#ifdef GRAPH_BIPARTITIONING_TEST
	std::cout << "----" << std::endl;

#ifdef AMP_STEALING_DEQUE_TEST
//	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>::WithTaskStorage<YourImplementation>,
//							BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>,
							BBGraphBipartitioning>();

#elif AMP_QUEUE_STACK_TEST

	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>,
						BBGraphBipartitioning>();

	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>::WithTaskStorage<GlobalLockQueue>,
						BBGraphBipartitioning>();

	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>,
						BBGraphBipartitioning>();

#elif AMP_SKIPLIST_TEST
	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>::WithPriorityTaskStorage<GlobalLockMultiSetPriorityQueue>,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>,
						BBGraphBipartitioning>();


#elif AMP_LOCK_TEST

	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>::WithMutex<TASLock>,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<CentralizedScheduler>::WithMutex<TTASLock>,
						BBGraphBipartitioning>();
	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>,
						BBGraphBipartitioning>();

#else
	this->run_partitioner<	Pheet::WithScheduler<StrategyScheduler2>,
							PPoPPBBGraphBipartitioning2<>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<StrategyScheduler2>,
							PPoPPBBGraphBipartitioning2<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT>();

	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<DistKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<DistKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<CentralKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<CentralKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<cpp11::CentralKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::BT>();
	this->run_partitioner<	Pheet::WithScheduler<BStrategyScheduler>::WithTaskStorage<cpp11::CentralKStrategyTaskStorage>,
							PPoPPBBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT>();

	this->run_partitioner<  Pheet::WithScheduler<StrategyScheduler>,
							PPoPPBBGraphBipartitioning<>
								::BT >();
	this->run_partitioner<  Pheet::WithScheduler<StrategyScheduler>,
							PPoPPBBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT >();

	this->run_partitioner<	Pheet::WithScheduler<BasicScheduler>,
							BBGraphBipartitioning>();
	this->run_partitioner<  Pheet::WithScheduler<BasicScheduler>,
							BBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT >();

	this->run_partitioner<	Pheet::WithScheduler<SynchroneousScheduler>,
							BBGraphBipartitioning>();
	this->run_partitioner<  Pheet::WithScheduler<SynchroneousScheduler>,
							BBGraphBipartitioning<>
								::WithLogic<BBGraphBipartitioningFREELogic>
								::BT >();

#endif

#endif
}

}

