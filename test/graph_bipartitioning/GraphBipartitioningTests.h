/*
 * GraphBipartitioningTests.h
 *
 *  Created on: 07.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef GRAPHBIPARTITIONINGTESTS_H_
#define GRAPHBIPARTITIONINGTESTS_H_

#include "../init.h"
#ifdef GRAPH_BIPARTITIONING_TEST
#include "GraphBipartitioningTest.h"
#endif
#include <algorithm>

/*
 *
 */
namespace pheet {

class GraphBipartitioningTests {
public:
	GraphBipartitioningTests();
	~GraphBipartitioningTests();

	void run_test();

private:
	template<class Pheet, template <class P> class Partitioner>
	void run_partitioner();
};

template <class Pheet, template <class P> class Partitioner>
void GraphBipartitioningTests::run_partitioner() {
#ifdef GRAPH_BIPARTITIONING_TEST
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t t = 0; t < sizeof(graph_bipartitioning_test_types)/sizeof(graph_bipartitioning_test_types[0]); t++) {
		for(size_t pr = 0; pr < sizeof(graph_bipartitioning_test_problems)/sizeof(graph_bipartitioning_test_problems[0]); pr++) {
			bool max_processed = false;
			procs_t cpus;
			for(size_t c = 0; c < sizeof(graph_bipartitioning_test_cpus)/sizeof(graph_bipartitioning_test_cpus[0]); c++) {
				cpus = graph_bipartitioning_test_cpus[c];
				if (mm.supports_SMT()) // with SMT we only want to run with all cores
				{
					cpus = max_cpus;
					c = sizeof(graph_bipartitioning_test_cpus); // break the loop on next iteration
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
				for(size_t s = 0; s < sizeof(graph_bipartitioning_test_seeds)/sizeof(graph_bipartitioning_test_seeds[0]); s++) {
					GraphBipartitioningTest<Pheet, Partitioner> gbt(cpus, graph_bipartitioning_test_types[t],
							graph_bipartitioning_test_problems[pr].n,
							graph_bipartitioning_test_problems[pr].p,
							graph_bipartitioning_test_problems[pr].max_w,
							graph_bipartitioning_test_seeds[s]);
					gbt.run_test();
				}
			}
		}
	}
#endif
}

}

#endif /* GRAPHBIPARTITIONINGTESTS_H_ */
