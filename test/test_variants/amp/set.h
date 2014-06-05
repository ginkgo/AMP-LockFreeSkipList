/*
 * set.h
 *
 *  Created on: May 01, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef AMP_SET_H_
#define AMP_SET_H_

#include "pheet/misc/types.h"

namespace pheet {

#define SET_BENCH true
const procs_t set_bench_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int set_bench_seeds[] = {0};
const SetBenchProblem set_bench_problems[] = {
		// n, p, max_w
		{10000, 1000, 0.99, 0.005},
		{10000, 1000, 0.5, 0.25}
};

}

#endif
