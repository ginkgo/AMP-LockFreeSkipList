/*
 * PrefixSumTests.h
 *
 *  Created on: Jun 28, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PREFIXSUMTESTS_H_
#define PREFIXSUMTESTS_H_

#include "../init.h"
#include "../Test.h"
#ifdef PREFIX_SUM_TEST
#include "PrefixSumTest.h"
#endif

namespace pheet {

class PrefixSumTests : Test {
public:
	PrefixSumTests();
	virtual ~PrefixSumTests();

	void run_test();

private:
	template<class Pheet, template <class P> class Algorithm>
	void run_prefix_sum();
};

template <class Pheet, template <class P> class Algorithm>
void PrefixSumTests::run_prefix_sum() {
#ifdef PREFIX_SUM_TEST
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t p = 0; p < sizeof(prefix_sum_num_problems)/sizeof(prefix_sum_num_problems[0]); p++) {
		for(size_t t = 0; t < sizeof(prefix_sum_test_types)/sizeof(prefix_sum_test_types[0]); t++) {
			for(size_t n = 0; n < sizeof(prefix_sum_test_n)/sizeof(prefix_sum_test_n[0]); n++) {
				bool max_processed = false;
				procs_t cpus;
				for(size_t c = 0; c < sizeof(prefix_sum_test_cpus)/sizeof(prefix_sum_test_cpus[0]); c++) {
					cpus = prefix_sum_test_cpus[c];
					if (mm.supports_SMT()) // with SMT we only want to run with all cores
					{
						cpus = max_cpus;
						c = sizeof(prefix_sum_test_cpus); // break the loop on next iteration
					}
					if (cpus > max_cpus)
						continue;
					if(cpus >= max_cpus) {
						if(!max_processed) {
							cpus = max_cpus;
							max_processed = true;
						}
						else {
							continue;
						}
					}
					for(size_t s = 0; s < sizeof(prefix_sum_test_seeds)/sizeof(prefix_sum_test_seeds[0]); s++) {
						PrefixSumTest<Pheet, Algorithm> st(cpus, prefix_sum_num_problems[p], prefix_sum_test_types[t], prefix_sum_test_n[n], prefix_sum_test_seeds[s]);
						st.run_test();
					}
				}
			}
		}
	}
#endif
}

} /* namespace pheet */
#endif /* PREFIXSUMTESTS_H_ */
