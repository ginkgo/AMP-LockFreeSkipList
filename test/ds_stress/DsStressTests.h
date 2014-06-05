/*
 * DsStressTests.h
 *
 *  Created on: Apr 30, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef DSSTRESSTESTS_H_
#define DSSTRESSTESTS_H_

#include "../init.h"


namespace pheet {

class DsStressTests {
public:
	DsStressTests();
	~DsStressTests();

	void run_test();

private:
	template<class Pheet, template <class, typename> class Ds, template <class, template <class, typename> class> class DsTest>
	void stress_test() {
#ifdef DS_STRESS_TEST
		typename Pheet::MachineModel mm;
		procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

		for(size_t t = 0; t < sizeof(ds_stress_test_types)/sizeof(ds_stress_test_types[0]); t++) {
			bool max_processed = false;
			for(size_t c = 0; c < sizeof(ds_stress_test_cpus)/sizeof(ds_stress_test_cpus[0]); c++) {
				procs_t cpus = ds_stress_test_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				for(unsigned int seed = ds_stress_test_seed_first; seed <= ds_stress_test_seed_last; ++seed) {
					DsTest<Pheet, Ds> dsst(cpus, ds_stress_test_types[t],
								seed);
						dsst.run_test();
				}
			}
		}
#endif
	}

	template<class Pheet, template <class> class Ds, template <class, template <class> class> class DsTest>
	void stress_test() {
#ifdef DS_STRESS_TEST
		typename Pheet::MachineModel mm;
		procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

		for(size_t t = 0; t < sizeof(ds_stress_test_types)/sizeof(ds_stress_test_types[0]); t++) {
			bool max_processed = false;
			for(size_t c = 0; c < sizeof(ds_stress_test_cpus)/sizeof(ds_stress_test_cpus[0]); c++) {
				procs_t cpus = ds_stress_test_cpus[c];
				if(cpus >= max_cpus) {
					if(!max_processed) {
						cpus = max_cpus;
						max_processed = true;
					}
					else {
						continue;
					}
				}
				for(unsigned int seed = ds_stress_test_seed_first; seed <= ds_stress_test_seed_last; ++seed) {
					DsTest<Pheet, Ds> dsst(cpus, ds_stress_test_types[t],
								seed);
						dsst.run_test();
				}
			}
		}
#endif
	}

};

} /* namespace pheet */
#endif /* DSSTRESSTESTS_H_ */
