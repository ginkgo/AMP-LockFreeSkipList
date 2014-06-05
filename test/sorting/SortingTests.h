/*
 * QuicksortTest.h
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SORTINGTESTS_H_
#define SORTINGTESTS_H_

#include "../init.h"
#include "../Test.h"
#ifdef SORTING_TEST
#include "SortingTest.h"
#endif

namespace pheet {

class SortingTests : Test {
public:
	SortingTests();
	virtual ~SortingTests();

	void run_test();

private:
	template<class Pheet, template <class P> class Sorter>
	void run_sorter();
};

template <class Pheet, template <class P> class Sorter>
void SortingTests::run_sorter() {
#ifdef SORTING_TEST
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t t = 0; t < sizeof(sorting_test_types)/sizeof(sorting_test_types[0]); t++) {
		for(size_t n = 0; n < sizeof(sorting_test_n)/sizeof(sorting_test_n[0]); n++) {
			bool max_processed = false;
			procs_t cpus;
			for(size_t c = 0; c < sizeof(sorting_test_cpus)/sizeof(sorting_test_cpus[0]); c++) {
				cpus = sorting_test_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				for(size_t s = 0; s < sizeof(sorting_test_seeds)/sizeof(sorting_test_seeds[0]); s++) {
					SortingTest<Pheet, Sorter> st(cpus, sorting_test_types[t], sorting_test_n[n], sorting_test_seeds[s]);
					st.run_test();
				}
			}
		}
	}
#endif
}

}

#endif /* SORTINGTESTS_H_ */
