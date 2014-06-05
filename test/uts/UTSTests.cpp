/*
* UTSTests.cpp
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#include "UTSTests.h"
#ifdef UTS_TEST
#include "UTSTest.h"
#include "RecursiveSearch/UTSRun.h"
#include "Strategy2/Strategy2UTS.h"

#include <pheet/sched/Basic/BasicScheduler.h>
#include <pheet/sched/Strategy2/StrategyScheduler2.h>
#include <pheet/sched/Synchroneous/SynchroneousScheduler.h>
#endif

//#include "../test_schedulers.h"
#include <iostream>


namespace pheet {

	const std::string uts_test_standardworkloads_params[] = {"T1 -t 1 -a 3 -d 10 -b 4 -r 19",
												"T5 -t 1 -a 0 -d 20 -b 4 -r 34",
												"T2 -t 1 -a 2 -d 16 -b 6 -r 502",
												"T3 -t 0 -b 2000 -q 0.124875 -m 8 -r 42",
												"T4 -t 2 -a 0 -d 16 -b 6 -r 1 -q 0.234375 -m 4 -r 1",
												"T1L -t 1 -a 3 -d 13 -b 4 -r 29",
												"T2L -t 1 -a 2 -d 23 -b 7 -r 220",
												"T3L -t 0 -b 2000 -q 0.200014 -m 5 -r 7",
												"T1XL -t 1 -a 3 -d 15 -b 4 -r 29"};

	template <class Pheet, template <class> class Test>
	void UTSTests::test()
	{
#ifdef UTS_TEST
		typename Pheet::MachineModel mm;
		procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

		for(size_t s = 0; s < sizeof(uts_test_standardworkloads)/sizeof(uts_test_standardworkloads[0]); s++) {
			bool max_processed = false;
			procs_t cpus;
			for(size_t c = 0; c < sizeof(uts_test_cpus)/sizeof(uts_test_cpus[0]); c++) {
				cpus = uts_test_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				for(size_t i = 0; i < uts_test_repetitions; ++i) {
					UTSTest<Test<Pheet> > iart(cpus,uts_test_standardworkloads_params[uts_test_standardworkloads[s]]);
					iart.run_test();
				}
			}
		}
#endif
	}

	void UTSTests::run_test()
	{
#ifdef UTS_TEST
		test<Pheet::WithScheduler<StrategyScheduler2>,
			 Strategy2UTS>();
		test<Pheet::WithScheduler<StrategyScheduler2>,
			 UTSRun>();
		test<Pheet::WithScheduler<BasicScheduler>,
			 UTSRun>();
		test<Pheet::WithScheduler<SynchroneousScheduler>,
			 UTSRun>();
#endif
	}

}
