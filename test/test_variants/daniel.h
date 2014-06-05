/*
 * full.h
 *
 *  Created on: 11.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef FULL_H_
#define FULL_H_

#include "pheet/misc/types.h"

namespace pheet {

  //#define SORTING_TEST true
const procs_t sorting_test_cpus[] = {1, 2, 4, 8, 16, 32, 64, 128};
const unsigned int sorting_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const size_t sorting_test_n[] = {10000000, 100000000, 1000000000, 8388607, 33554431, 134217727};
const int sorting_test_types[] = {0, 1, 3, 4};

//#define GRAPH_BIPARTITIONING_TEST true
const procs_t graph_bipartitioning_test_cpus[] = {1, 2, 4, 8, 16, 32, 64, 128};
const unsigned int graph_bipartitioning_test_seeds[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

const GraphBipartitioningProblem graph_bipartitioning_test_problems[] = {
		// n, p, max_w
		{38, 0.5, 1},
		{38, 0.5, 10},
		{38, 0.5, 1000},
		{38, 0.9, 1},
		{38, 0.9, 10},
		{38, 0.9, 1000},
		{40, 0.5, 1},
		{40, 0.5, 10},
		{40, 0.5, 1000},
		{40, 0.9, 1},
		{40, 0.9, 10},
		{40, 0.9, 1000},
		{42, 0.5, 1},
		{42, 0.5, 10},
		{42, 0.5, 1000},
		{42, 0.9, 10},
		{42, 0.9, 1000},
		{44, 0.1, 1},
		{44, 0.1, 10},
		{44, 0.1, 1000},
		{44, 0.5, 1},
		{44, 0.5, 10},
		{44, 0.5, 1000},
		{44, 0.9, 10},
		{44, 0.9, 1000},
		{46, 0.1, 1},
		{46, 0.1, 10},
		{46, 0.1, 1000},
		{46, 0.5, 10},
		{46, 0.5, 1000},
		{46, 0.9, 1000},
		{48, 0.1, 1},
		{48, 0.1, 10},
		{48, 0.1, 1000},
		{48, 0.5, 1000},
		{48, 0.9, 1000},
		{50, 0.1, 1},
		{50, 0.1, 10},
		{50, 0.1, 1000},
		{50, 0.5, 1000},
		{54, 0.1, 1},
		{54, 0.1, 10},
		{54, 0.1, 1000},
		{58, 0.1, 1},
		{58, 0.1, 10},
		{58, 0.1, 1000}
};
//const size_t graph_bipartitioning_test_n[] = {44, 46, 48};
//const double graph_bipartitioning_test_p[] = {0.1, 0.5, 0.9};
//const double graph_bipartitioning_test_max_w[] = {1, 10, 1000};
const int graph_bipartitioning_test_types[] = {0};

//#define GRAPHBIANDLUPIV_TEST true

//#define INAROW_TEST true
const procs_t inarow_test_cpus[] = {1, 2, 4, 8, 16, 32, 64, 128};
const unsigned int inarow_test_lookaheads[] = {6, 7, 8};

const bool    nqueens_test        = false;
const procs_t nqueens_test_cpus[] = {1, 2, 4, 8, 16, 32, 64, 128};
const size_t  nqueens_test_n[]    = {24, 32, 48};

//#define UTS_TEST      true
const procs_t uts_test_cpus[] = {4};

// 0 # (T1) Geometric [fixed] ------- Tree size = 4130071, tree depth = 10, num leaves = 3305118 (80.03%)
// 1 # (T5) Geometric [linear dec.] - Tree size = 4147582, tree depth = 20, num leaves = 2181318 (52.59%)
// 2 # (T2) Geometric [cyclic] ------ Tree size = 4117769, tree depth = 81, num leaves = 2342762 (56.89%)
// 3 # (T3) Binomial ---------------- Tree size = 4112897, tree depth = 1572, num leaves = 3599034 (87.51%)
// 4 # (T4) Hybrid ------------------ Tree size = 4132453, tree depth = 134, num leaves = 3108986 (75.23%)
// 5 # (T1L) Geometric [fixed] ------ Tree size = 102181082, tree depth = 13, num leaves = 81746377 (80.00%)
// 6 # (T2L) Geometric [cyclic] ----- Tree size = 96793510, tree depth = 67, num leaves = 53791152 (55.57%)
// 7 # (T3L) Binomial --------------- Tree size = 111345631, tree depth = 17844, num leaves = 89076904 (80.00%)
// 8 # (T1XL) Geometric [fixed] ----- Tree size = 1635119272, tree depth = 15, num leaves = 1308100063 (80.00%)
const unsigned int uts_test_standardworkloads[] = {0,1,2,3,4,5,6,7,8};

#define TriStrip_TEST true
const procs_t tristrip_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
const procs_t tristrip_test_nodecount[] = {1024*12};

#define SOR_TEST	true
const procs_t sor_test_cpus[] = {1, 2, 3, 6, 12, 24, 48};
 const int sor_test_maxtrissize[] = {1536};
const int sor_test_iterations[] =  {100};
const int sor_test_slices[] = {48};
const double sor_test_omega[] = {1.25};
const bool sor_prio[] = {true,false};
 const bool sor_runuts = true;
 const bool sor_runsor = true;
 const bool sor_usestrat = true;


#define LUPIV_TEST      true
 const procs_t lupiv_test_cpus[] = {2,4,6,8,10,12};
 const unsigned int lupiv_test_seeds[] = {0};
 const size_t lupiv_test_n[] = { 1536};
 const int lupiv_test_types[] = {0};


}

#endif /* FULL_H_ */
