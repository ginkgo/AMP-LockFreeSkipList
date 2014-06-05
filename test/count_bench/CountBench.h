/*
 * CountBench.h
 *
 *  Created on: 01.05.2013
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef COUNTBENCH_H_
#define COUNTBENCH_H_

#include "../init.h"
#include "../Test.h"
#ifdef COUNT_BENCH
#include "CountTest.h"
#endif

namespace pheet {

class CountBench : Test {
public:
	CountBench();
	~CountBench();

	void run_test();

private:
	template<class Pheet, template <class, typename> class Count, template <class, class> class Benchmark>
	void run_bench();
};


template <class Pheet, template <class, typename> class Count, template <class, class> class Benchmark>
void CountBench::run_bench() {
#ifdef COUNT_BENCH
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t p = 0; p < sizeof(count_bench_p)/sizeof(count_bench_p[0]); p++) {
		for(size_t n = 0; n < sizeof(count_bench_n)/sizeof(count_bench_n[0]); n++) {
			bool max_processed = false;
			procs_t cpus;
			for(size_t c = 0; c < sizeof(count_bench_cpus)/sizeof(count_bench_cpus[0]); c++) {
				cpus = count_bench_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				for(size_t s = 0; s < sizeof(count_bench_seeds)/sizeof(count_bench_seeds[0]); s++) {
					CountTest<Pheet, Count, Benchmark> gbt(cpus,
							count_bench_n[n],
							count_bench_p[p],
							count_bench_seeds[s]);
					gbt.run_test();
				}
			}
		}
	}

#endif
}
} /* namespace context */
#endif /* COUNTBENCH_H_ */
