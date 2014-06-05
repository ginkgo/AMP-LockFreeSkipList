/*
 * mtaap2013.h
 *
 *  Created on: Dec 12, 2012
 *      Author: Manuel Pöter
 *	   License: Boost Software License 1.0
 */

#ifndef MTAAP2013_H_
#define MTAAP2013_H_

#include "pheet/misc/types.h"

namespace pheet {

#ifdef __MIC__
#define NUMBER_OF_CORES {1, 2, 3, 6, 12, 16, 24, 32, 48, 61}
#else
#define NUMBER_OF_CORES {1, 2, 3, 6, 12, 16, 24, 32, 48}
#endif

#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = NUMBER_OF_CORES;
const unsigned int graph_bipartitioning_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
        {35, 0.9, 1000},
        {39, 0.5, 1},
};
const int graph_bipartitioning_test_types[] = {0};


#define PREFIX_SUM_TEST	true
const procs_t prefix_sum_test_cpus[] = NUMBER_OF_CORES;
const unsigned int prefix_sum_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t prefix_sum_num_problems[] = {1};
const size_t prefix_sum_test_n[] = {16 * 100000000};
const int prefix_sum_test_types[] = {0};
}

#endif /* MTAAP2013_H_ */
