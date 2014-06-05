/*
 * CountBench.cpp
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */


#include "../init.h"

#include "CountBench.h"
#ifdef COUNT_BENCH
#include <pheet/primitives/Counter/Simple/SimpleCounter.h>
#include <pheet/primitives/Reducer/Sum/SumReducer.h>
#include "CountBenchTask.h"
#include "CountBenchHyperTask.h"
#endif

namespace pheet {

#ifdef COUNT_BENCH
template <class Pheet, typename T>
using SumReducerCounter = SumReducer<Pheet, T>;
#endif

CountBench::CountBench() {
	// TODO Auto-generated constructor stub

}

CountBench::~CountBench() {
	// TODO Auto-generated destructor stub
}


void CountBench::run_test() {
#ifdef COUNT_BENCH
	std::cout << "----" << std::endl;

	// default tests
	this->run_bench<	Pheet,
						SimpleCounter,
						CountBenchTask>();
	this->run_bench<	Pheet,
						SumReducerCounter,
						CountBenchHyperTask>();
#endif
}

} /* namespace context */
