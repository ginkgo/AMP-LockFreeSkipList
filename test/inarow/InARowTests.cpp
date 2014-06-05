/*
* InARowTests.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#include "InARowTests.h"
#ifdef INAROW_TEST
#include "InARowTest.h"
#include "RecursiveSearch/InARowGame.h"
#endif

#include <pheet/sched/Strategy/StrategyScheduler.h>

#include <iostream>
#include <algorithm>

namespace pheet {

#ifdef INAROW_TEST
	const unsigned int scenario1[] = {2,4,3,1,1,2,3,4,5,4,1,0};
	const unsigned int scenario2[] = {3,2,1,1,4,1,2,4,3,4,4,3,1,5,6,7,0};
#endif

	template <class Test>
	void InARowTests::test(unsigned int width, unsigned int height, unsigned int rowlength, unsigned int* scenario)
	{
#ifdef INAROW_TEST
		typename Pheet::MachineModel mm;
		procs_t max_cpus = mm.get_num_leaves();
				//std::min(mm.get_num_leaves(), Test::max_cpus);

			printf("Maxcpus: %lu\n",max_cpus);

		for(size_t la = 0; la < sizeof(inarow_test_lookaheads)/sizeof(inarow_test_lookaheads[0]); la++) {
			bool max_processed = false;
			procs_t cpus;
			for(size_t c = 0; c < sizeof(inarow_test_cpus)/sizeof(inarow_test_cpus[0]); c++) {
				cpus = inarow_test_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				InARowTest<Test> iart(cpus, width, height, rowlength, inarow_test_lookaheads[la], scenario);
				iart.run_test();
			}
		}
#endif
	}

	void InARowTests::run_test()
	{
#ifdef INAROW_TEST
		if(inarow_test) {

			//test<DefaultBasicScheduler>(8,8,4,4,2,(unsigned int*)scenario2);
			//test<DefaultBasicScheduler>(8,8,4,7,8,(unsigned int*)scenario2);

			//test<DefaultSynchroneousScheduler>(8,8,4,10,1,(unsigned int*)scenario1);
			//		return;
			//test<DefaultBasicScheduler>(8,8,4,7,8,(unsigned int*)scenario2);

			test<InARowGame<Pheet::WithScheduler<StrategyScheduler> > >(8,8,4,(unsigned int*)scenario2);
	//		test<InARowGame<PrimitiveHeapPriorityScheduler> >(8,8,4,(unsigned int*)scenario2);
		//	test<DefaultBasicScheduler>(8,8,4,7,8,(unsigned int*)scenario2);
		}
#endif
	}

}
