/*
 * SetBench.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef SETBENCH_H_
#define SETBENCH_H_

#include "../init.h"
#include "../Test.h"
#ifdef SET_BENCH
#include "SetTest.h"
#endif

namespace pheet {

class SetBench : Test {
public:
	SetBench();
	~SetBench();

	void run_test();

private:
	template<class Pheet, template <class, typename, class> class Set>
	void run_bench();
};


template <class Pheet, template <class, typename, class> class Set>
void SetBench::run_bench() {
#ifdef SET_BENCH
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t pr = 0; pr < sizeof(set_bench_problems)/sizeof(set_bench_problems[0]); pr++) {
		bool max_processed = false;
		procs_t cpus;
		for(size_t c = 0; c < sizeof(set_bench_cpus)/sizeof(set_bench_cpus[0]); c++) {
			cpus = set_bench_cpus[c];
			if(cpus >= max_cpus) {
				if(!max_processed) {
					cpus = max_cpus;
					max_processed = true;
				}
				else {
					continue;
				}
			}
			for(size_t s = 0; s < sizeof(set_bench_seeds)/sizeof(set_bench_seeds[0]); s++) {
				SetTest<Pheet, Set> gbt(cpus,
						set_bench_problems[pr].range,
						set_bench_problems[pr].blocks,
						set_bench_problems[pr].contains_p,
						set_bench_problems[pr].add_p,
						set_bench_seeds[s]);
				gbt.run_test();
			}
		}
	}

#endif
}
} /* namespace context */
#endif /* SETBENCH_H_ */
