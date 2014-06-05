/*
 * pheet_tests.cpp
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#define NDEBUG 1

#include "init.h"
#include "ds_stress/DsStressTests.h"
#include "sorting/SortingTests.h"
#include "graph_bipartitioning/GraphBipartitioningTests.h"
#include "lupiv/LUPivTests.h"
#include "inarow/InARowTests.h"
#include "n-queens/NQueensTests.h"
#include "lupiv/LUPivTests.h"
#include "uts/UTSTests.h"
//#include "sor/SORTests.h"
#include "tristrip/TriStripTests.h"
#include "prefix_sum/PrefixSumTests.h"
#include "sssp/SsspTests.h"
#include "set_bench/SetBench.h"
#include "count_bench/CountBench.h"

using namespace pheet;

int main(int argc, char* argv[]) {
	DsStressTests dsst;
	dsst.run_test();

	SortingTests st;
	st.run_test();

  	GraphBipartitioningTests gpt;
  	gpt.run_test();
   
  	PrefixSumTests pt;
  	pt.run_test();

	InARowTests iarts;
	iarts.run_test();

	NQueensTests nqt;
	nqt.run_test();

	UTSTests utss;
	utss.run_test();

	SsspTests sssp;
	sssp.run_test();

	SetBench sb;
	sb.run_test();

	CountBench cb;
	cb.run_test();

	TriStripTests tristrip;
	tristrip.run_test();

	return 0;
}
