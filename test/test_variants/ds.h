/*
 * ds.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef DS_H_
#define DS_H_

#include "pheet/misc/types.h"

namespace pheet {

    
#define DS_STRESS_TEST true
    const procs_t ds_stress_test_cpus[] = {1, 2, 4, 8, 12, 16, 24, 32, 48};
// 0: Throughput, 1: Correctness
const int ds_stress_test_types[] = {0};
const unsigned int ds_stress_test_seed_first = 0;
const unsigned int ds_stress_test_seed_last = 0;
    const unsigned int ds_stress_test_prefill[] = {10000};
// 0: 50:50, 1: 50:50d
const unsigned int ds_stress_test_linear_dist[] = {0, 1};

// Number of operations per thread in correctness test for Queue, Stack, PQ and Bag (work in progress)
const size_t ds_stress_test_linear_correctness_n[] = {1000};
// Percentage of push calls before the first pop
const double ds_stress_test_linear_correctness_prefill_p[] = {0.2};

// 0: 1:1:2, 1: 1:1:2d, 2: 1:1:98, 3: 1:1:P-2d
const unsigned int ds_stress_test_set_dist[] = {0, 1};
    const unsigned int ds_stress_test_set_max_v[] = {20000};

const size_t ds_stress_test_num_locks[] = {2, 4, 8, 16};
// 0: single access, 1: random subarray (uniform), 2: random subarray (high congestion)
const int ds_stress_test_lock_pattern[] = {0, 1, 2};


}


#endif /* DS_H_ */
