/*
 * stealing_deque.h
 *
 *  Created on: May 30, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef AMP_STEALING_DEQUE_H_
#define AMP_STEALING_DEQUE_H_

namespace pheet {

#define AMP_STEALING_DEQUE_TEST true


// Debug configuration
#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 6, 12, 48};
const unsigned int sorting_test_seeds[] = {0};
const size_t sorting_test_n[] = {5000000};
const int sorting_test_types[] = {0};

#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = {1, 2, 6, 12, 48};
const unsigned int graph_bipartitioning_test_seeds[] = {0};
const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
		{35, 0.5, 1},
		{37, 0.5, 1000},
};
const int graph_bipartitioning_test_types[] = {0};

/*
// The real thing
#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 3, 6, 12, 24, 48, 96};
const unsigned int sorting_test_seeds[] = {0, 1, 2, 3, 4};
const size_t sorting_test_n[] = {5000000, 50000000};
const int sorting_test_types[] = {0};

#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = {1, 2, 3, 6, 12, 24, 48, 96};
const unsigned int graph_bipartitioning_test_seeds[] = {0, 1, 2, 3, 4};
const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
		{35, 0.5, 1},
		{37, 0.5, 1000},
		{40, 0.5, 1},
		{42, 0.5, 1000},
};
const int graph_bipartitioning_test_types[] = {0};
*/
}


#endif /* AMP_STEALING_DEQUE_H_ */
