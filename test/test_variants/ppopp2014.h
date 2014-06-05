/*
 *  ppopp2014.h
 *
 *  Created on: 14.08.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPP2014_H_
#define PPOPP2014_H_

#include "pheet/misc/types.h"

namespace pheet {

#define SSSP_TEST true
// Run tests with following numbers of CPUs (capped at max cpus)
const procs_t sssp_test_cpus[] = {1, 2, 3, 5, 10, 20, 40, 80};
// Run tests with following random seeds for graph generator (to run multiple times, just add more random seeds)
const unsigned int sssp_test_seeds[] = {0/*, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19*/};
const GraphBipartitioningProblem sssp_test_problems[] = {
		// nodes, edge probability, maximum edge weight (integer, minimum weight is always 1)
		{10000, 0.5, 100000000},
//		{50000, 0.001, 100000000},
};
// Which type of graph is used (so far only 0, erdos-renyi random graphs are supported)
const int sssp_test_types[] = {0};
// Which values for k to use (only for  data structures that support k)
const size_t sssp_test_k[] = {/*0, 1, 2, 4, 8, 16, 32, 64, 128, 256,*/ 512/*, 1024, 2048, 4096, 8192, 16384, 32768*/};


// Configuration for simulator - not used for normal runs
//#define SSSP_SIM true
const size_t sssp_sim_k = 1000;
const size_t sssp_sim_p = 200;

}

#endif /* PPOPP2014_H_ */

