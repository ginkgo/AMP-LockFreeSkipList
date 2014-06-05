/*
 * count.h
 *
 *  Created on: May 01, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef AMP_COUNT_H_
#define AMP_COUNT_H_

#include "pheet/misc/types.h"

namespace pheet {

#define COUNT_BENCH true
const procs_t count_bench_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int count_bench_seeds[] = {0};
const size_t count_bench_n[] = {10000};
const double count_bench_p[] = {0.7};

}

#endif
