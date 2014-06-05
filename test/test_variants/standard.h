/*
 * full.h
 *
 *  Created on: 11.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SHORT_SINGLE_H_
#define SHORT_SINGLE_H_

#include "pheet/misc/types.h"

namespace pheet {

#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 4, 8};
const unsigned int sorting_test_seeds[] = {0};
const size_t sorting_test_n[] = {10000000};
const int sorting_test_types[] = {0};

#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = {1, 2, 4, 8};
const unsigned int graph_bipartitioning_test_seeds[] = {0};
const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
		{35, 0.5, 1000}
};
const int graph_bipartitioning_test_types[] = {0};

//#define INAROW_TEST true
const procs_t inarow_test_cpus[] = {1, 2, 4, 8};
const unsigned int inarow_test_lookaheads[] = {5};

//#define NQUEENS_TEST true
const procs_t nqueens_test_cpus[] = {1, 2, 4, 8};
const size_t  nqueens_test_n[]    = {24};

//#define LUPIV_TEST	true
const procs_t lupiv_test_cpus[] = {1, 2, 4, 8};
const unsigned int lupiv_test_seeds[] = {0};
const size_t lupiv_test_n[] = {1024};
const int lupiv_test_types[] = {0};

#define PREFIX_SUM_TEST true
const procs_t prefix_sum_test_cpus[] = {1, 2, 4, 8};
const unsigned int prefix_sum_test_seeds[] = {0};
const size_t prefix_sum_num_problems[] = {1};
const size_t prefix_sum_test_n[] = {1000000};
const int prefix_sum_test_types[] = {0};

#define SSSP_TEST true
const procs_t sssp_test_cpus[] = {1, 2, 4, 8};
const unsigned int sssp_test_seeds[] = {0}; //, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
const GraphBipartitioningProblem sssp_test_problems[] = {
		// n, p, max_w
		{3000, 0.5, 100000000},
};
const int sssp_test_types[] = {0};
const size_t sssp_test_k[] = {1024};// {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};

//#define SSSP_SIM true
//#define SSSP_SIM_STRUCTURED true
//#define SSSP_SIM_DEP_CHECK true
//#define SSSP_SIM_VERIFY_THEORY true
const size_t sssp_sim_k = 1000;
const size_t sssp_sim_p = 200;

const double sssp_sim_theory_p = 0.1;
const double sssp_sim_theory_div = 100000000;

#define UTS_TEST      true
const procs_t uts_test_cpus[] = {1, 2, 4, 8};
const size_t uts_test_repetitions = 1;

// 0 # (T1) Geometric [fixed] ------- Tree size = 4130071, tree depth = 10, num leaves = 3305118 (80.03%)
// 1 # (T5) Geometric [linear dec.] - Tree size = 4147582, tree depth = 20, num leaves = 2181318 (52.59%)
// 2 # (T2) Geometric [cyclic] ------ Tree size = 4117769, tree depth = 81, num leaves = 2342762 (56.89%)
// 3 # (T3) Binomial ---------------- Tree size = 4112897, tree depth = 1572, num leaves = 3599034 (87.51%)
// 4 # (T4) Hybrid ------------------ Tree size = 4132453, tree depth = 134, num leaves = 3108986 (75.23%)
// 5 # (T1L) Geometric [fixed] ------ Tree size = 102181082, tree depth = 13, num leaves = 81746377 (80.00%)
// 6 # (T2L) Geometric [cyclic] ----- Tree size = 96793510, tree depth = 67, num leaves = 53791152 (55.57%)
// 7 # (T3L) Binomial --------------- Tree size = 111345631, tree depth = 17844, num leaves = 89076904 (80.00%)
// 8 # (T1XL) Geometric [fixed] ----- Tree size = 1635119272, tree depth = 15, num leaves = 1308100063 (80.00%)
const unsigned int uts_test_standardworkloads[] = {0, 3};


//#define SORANDUTS


}

#endif /* SHORT_SINGLE_H_ */
