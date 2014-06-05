/*
 * Strategy2Sssp.h
 *
 *  Created on: Oct 08, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2SSSP_H_
#define STRATEGY2SSSP_H_

#include "Strategy2SsspStrategy.h"
#include "Strategy2SsspPerformanceCounters.h"

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>
//#include <pheet/ds/StrategyTaskStorage/KLSMLocality2/KLSMLocalityTaskStorage2.h>
#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2SsspImpl : public Pheet::Task {
public:
	typedef Strategy2SsspImpl<Pheet, TaskStorageT> Self;
	typedef Strategy2SsspStrategy<Pheet, TaskStorageT> Strategy;
	typedef typename Strategy::TaskStorage TaskStorage;
	typedef Strategy2SsspPerformanceCounters<Pheet> PerformanceCounters;

	Strategy2SsspImpl(SsspGraphVertex* graph, size_t size, PerformanceCounters& pc)
	:graph(graph), node(0), distance(0), pc(pc) {
		pc.last_non_dead_time.start_timer();
		pc.last_task_time.start_timer();
		pc.last_update_time.start_timer();
	}
	Strategy2SsspImpl(SsspGraphVertex* graph, size_t node, size_t distance, PerformanceCounters& pc)
	:graph(graph), node(node), distance(distance), pc(pc) {}
	virtual ~Strategy2SsspImpl() {}

	virtual void operator()() {
		pc.last_task_time.take_time();
		size_t d = graph[node].distance.load(std::memory_order_relaxed);
		if(d != distance) {
			pc.num_dead_tasks.incr();
			// Distance has already been improved in the meantime
			// No need to check again
			return;
		}
		pc.num_actual_tasks.incr();
		pc.last_non_dead_time.take_time();
		for(size_t i = 0; i < graph[node].num_edges; ++i) {
			size_t new_d = d + graph[node].edges[i].weight;
			size_t target = graph[node].edges[i].target;
			size_t old_d = graph[target].distance.load(std::memory_order_relaxed);
			while(old_d > new_d) {
				if(graph[target].distance.compare_exchange_strong(old_d, new_d, std::memory_order_relaxed)) {
					pc.last_update_time.take_time();

					Pheet::template
						spawn_s<Self>(
								Strategy(new_d, graph[target].distance),
								graph, target, new_d, pc);
					break;
				}
			}
		}
	}

	static void set_k(size_t k) {
		Strategy::default_k = k;
	}

	static void print_name() {
		std::cout << name << "<";
		TaskStorage::print_name();
		std::cout << ">";
	}

	static char const name[];
private:
	SsspGraphVertex* graph;
	size_t node;
	size_t distance;
	PerformanceCounters pc;
};

template <class Pheet, template <class, class> class TaskStorageT>
char const Strategy2SsspImpl<Pheet, TaskStorageT>::name[] = "Strategy2 Sssp";

template <class Pheet>
using Strategy2Sssp = Strategy2SsspImpl<Pheet, KLSMLocalityTaskStorage >;

template <class Pheet>
using Strategy2SsspNoK = Strategy2SsspImpl<Pheet, LSMLocalityTaskStorage >;

/*
template <class Pheet>
using Strategy2SsspK2 = Strategy2SsspImpl<Pheet, KLSMLocalityTaskStorage2 >;
*/

} /* namespace pheet */
#endif /* STRATEGY2SSSP_H_ */
