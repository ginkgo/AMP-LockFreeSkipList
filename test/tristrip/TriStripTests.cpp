/*
 * TriStripTests.cpp
 *
 *  Created on: 5.12.2011
 *      Author: Daniel Cederman
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "TriStripTests.h"

#ifdef TriStrip_TEST

#include "TriStripTest.h"
#include "SGITriStrip/TriStripRun.h"

#include <pheet/sched/Strategy/StrategyScheduler.h>
#include <pheet/sched/Basic/BasicScheduler.h>

//#include "../test_schedulers.h"
#include <iostream>

#endif

namespace pheet {

template<class Pheet, template <class> class Test, bool Prio>
void TriStripTests::test() {
#ifdef TriStrip_TEST
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for (size_t nc = 0; nc< sizeof(tristrip_test_nodecount)/ sizeof(tristrip_test_nodecount[0]); nc++) {
//		for(size_t sl = 0; sl < sizeof(sor_test_slices)/sizeof(sor_test_slices[0]); sl++) {
//			for(size_t o = 0; o < sizeof(sor_test_omega)/sizeof(sor_test_omega[0]); o++) {
//				for(size_t ms = 0; ms < sizeof(sor_test_maxtrissize)/sizeof(sor_test_maxtrissize[0]); ms++) {
		bool max_processed = false;
		procs_t cpus;

		for (size_t c = 0;c < sizeof(tristrip_test_cpus) / sizeof(tristrip_test_cpus[0]);c++) {
			cpus = tristrip_test_cpus[c];
			if(cpus >= max_cpus) {
				if(!max_processed) {
					cpus = max_cpus;
					max_processed = true;
				}
				else {
					continue;
				}
			}

			for(size_t s = 0; s < sizeof(sssp_test_seeds)/sizeof(sssp_test_seeds[0]); s++) {
				unsigned int seed = sssp_test_seeds[s];
				TriStripTest<Pheet, Test> iart(cpus, tristrip_test_nodecount[nc], seed, Prio);
				iart.run_test();
			}
		}
	}
#endif
}

void TriStripTests::run_test() {
#ifdef TriStrip_TEST
	test<Pheet::WithScheduler<StrategyScheduler>,
		TriStripRun, true>();
	test<Pheet::WithScheduler<BasicScheduler>,
		TriStripRun, false>();
#endif
}

}
