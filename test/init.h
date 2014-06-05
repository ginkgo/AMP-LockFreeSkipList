/*
 * init.h
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PHEET_TEST_INIT_H_
#define PHEET_TEST_INIT_H_

#include "settings.h"
#include <pheet/pheet.h>

namespace pheet {
struct GraphBipartitioningProblem {
	size_t n;
	double p;
	size_t max_w;
};

struct SetBenchProblem {
	size_t range;
	size_t blocks;
	double contains_p;
	double add_p;
};

}

#include ACTIVE_TEST

namespace pheet {

#ifdef GRAPHBIANDLUPIV_TEST
bool const    graphbiandlupiv_test        = GRAPHBIANDLUPIV_TEST;
#else
bool const    graphbiandlupiv_test        = false;
#endif


#ifdef SORTING_TEST
bool const    sorting_test        = SORTING_TEST;
#else
bool const    sorting_test        = false;
#endif

#ifdef GRAPH_BIPARTITIONING_TEST
bool const    graph_bipartitioning_test        = GRAPH_BIPARTITIONING_TEST;
#else
bool const    graph_bipartitioning_test        = false;
#endif

#ifdef INAROW_TEST
bool const    inarow_test        = INAROW_TEST;
#else
bool const    inarow_test        = false;
#endif

#ifdef LUPIV_TEST
bool const    lupiv_test        = LUPIV_TEST;
#else
bool const    lupiv_test        = false;
#endif

#ifdef SOR_TEST
bool const    sor_test        = SOR_TEST;
#else
bool const    sor_test        = false;
#endif

#ifdef TriStrip_TEST
bool const    tristrip_test        = TriStrip_TEST;
#else
bool const    tristrip_test        = false;
#endif


#ifdef UTS_TEST
bool const    uts_test        = UTS_TEST;
#else
bool const    uts_test        = false;
#endif

#ifdef NQUEENS_TEST
bool const    nqueens_test        = NQUEENS_TEST;
#else
bool const    nqueens_test        = false;
#endif

}

#endif /* PHEET_TEST_INIT_H_ */
