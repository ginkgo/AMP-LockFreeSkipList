/*
 * DsStressTests.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#include "DsStressTests.h"

#include <pheet/ds/Queue/GlobalLock/GlobalLockQueue.h>
#include <pheet/ds/Stack/GlobalLock/GlobalLockStack.h>
#include <pheet/ds/PriorityQueue/GlobalLockHeap/GlobalLockHeap.h>
#include <pheet/ds/Set/GlobalLock/GlobalLockSet.h>

#include <pheet/primitives/Mutex/TASLock/TASLock.h>
#include <pheet/primitives/Mutex/TTASLock/TTASLock.h>
#include <pheet/primitives/Mutex/BackoffLock/BackoffLock.h>

#include <CoarseListSet.h>
#include <OptimisticListSet.h>
#include <LazyListSet.h>
#include <LockFreeListSet.h>
#include <LockFreeList.h>
#include <LockFreeSkipList.h>

#ifdef DS_STRESS_TEST
#include "linear/LinearDsTest.h"
#include "set/SetDsTest.h"
#include "lock/LockTest.h"
#endif

namespace pheet {

    template <class Pheet, typename TT>
    using GlobalLockHeapLess = GlobalLockHeap<Pheet, TT>;

    template <class Pheet, typename TT>
    using GlobalLockSetLess = GlobalLockSet<Pheet, TT>;

    DsStressTests::DsStressTests() {

    }

    DsStressTests::~DsStressTests() {
    }


    void DsStressTests::run_test() {
#ifdef DS_STRESS_TEST
        std::cout << "----" << std::endl;

        this->stress_test<Pheet, GlobalLockSetLess, SetDsTest>();
        this->stress_test<Pheet, CoarseListSet, SetDsTest>();
        this->stress_test<Pheet, OptimisticListSet, SetDsTest>();
        this->stress_test<Pheet, LazyListSet, SetDsTest>();
        this->stress_test<Pheet, LockFreeListSet, SetDsTest>();
        this->stress_test<Pheet, LockFreeList, SetDsTest>();
        this->stress_test<Pheet, LockFreeSkipList, SetDsTest>();

#endif
    }

} /* namespace pheet */
