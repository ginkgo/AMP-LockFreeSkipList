/*
 * SetBench.cpp
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */


#include "../init.h"

#include "SetBench.h"
#ifdef SET_BENCH
#include <pheet/ds/Set/GlobalLock/GlobalLockSet.h>
#endif

namespace pheet {

SetBench::SetBench() {
	// TODO Auto-generated constructor stub

}

SetBench::~SetBench() {
	// TODO Auto-generated destructor stub
}


void SetBench::run_test() {
#ifdef SET_BENCH
	std::cout << "----" << std::endl;

	// default tests
	this->run_bench<	Pheet,
						GlobalLockSet>();

#endif
}

} /* namespace context */
