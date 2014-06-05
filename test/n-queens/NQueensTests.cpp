/*
 * NQueensTests.cpp
 *
 *  Created on: 2011-10-11
 *      Author: Anders Gidenstam, Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "NQueensTests.h"

#ifdef NQUEENS_TEST
#include "ParallelRecursive/ParallelRecursiveNQueens.h"
#include <pheet/sched/Strategy/StrategyScheduler.h>

//#include "../test_schedulers.h"
#include <iostream>
#endif

namespace pheet {

NQueensTests::NQueensTests() {

}

NQueensTests::~NQueensTests() {

}

void NQueensTests::run_test() {
#ifdef NQUEENS_TEST
	std::cout << "----" << std::endl;

	//	this->run_solver<ParallelRecursiveNQueens<DefaultSynchroneousScheduler> >();
		this->run_solver<ParallelRecursiveNQueens<Pheet::WithScheduler<StrategyScheduler>>>();
     //           this->run_solver<ParallelRecursiveNQueens<DefaultBasicScheduler> >();
      //          this->run_solver<ParallelRecursiveNQueens<DefaultMixedModeScheduler> >();
       //         this->run_solver<ParallelRecursiveNQueens<PrimitiveHeapPriorityScheduler> >();
        //        this->run_solver<ParallelRecursiveNQueens<PrimitivePriorityScheduler> >();
         //       this->run_solver<ParallelRecursiveNQueens<FallbackPriorityScheduler> >(); // Problems here?
#endif
}

}
