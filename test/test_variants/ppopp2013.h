/*
 * ppopp2012.h
 *
 *  Created on: Jul 31, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PPOPP2013_H_
#define PPOPP2013_H_

#include "pheet/misc/types.h"

namespace pheet {

#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int sorting_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t sorting_test_n[] = {10000000};
const int sorting_test_types[] = {0, 1};

#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int graph_bipartitioning_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
        {31, 0.9, 1},
        {35, 0.9, 1000},
        {39, 0.5, 1},
        {43, 0.5, 1000},
        {53, 0.1, 1},
        {57, 0.1, 1000}
};
const int graph_bipartitioning_test_types[] = {0};

//#define LUPIV_TEST	true
const procs_t lupiv_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int lupiv_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t lupiv_test_n[] = {2048, 4096};
const int lupiv_test_types[] = {0};

#define PREFIX_SUM_TEST	true
const procs_t prefix_sum_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int prefix_sum_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t prefix_sum_num_problems[] = {1, 2, 6, 12};
const size_t prefix_sum_test_n[] = {200000000};
const int prefix_sum_test_types[] = {0};

#define SSSP_TEST true
const procs_t sssp_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const unsigned int sssp_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const GraphBipartitioningProblem sssp_test_problems[] = {
		// n, p, max_w
        {15000, 0.5, 1},
        {15000, 0.9, 1},
        {15000, 0.5, 1000},
        {15000, 0.9, 1000},
        {20000, 0.1, 1}
};
const int sssp_test_types[] = {0};
const size_t sssp_test_k[] = {1024}; //{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096}

}


#endif /* PPOPP2013_H_ */
