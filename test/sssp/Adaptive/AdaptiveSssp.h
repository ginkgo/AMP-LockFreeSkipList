/*
 * AdaptiveSssp.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef ADAPTIVESSSP_H_
#define ADAPTIVESSSP_H_

#include "AdaptiveSsspStrategy.h"
#include "AdaptiveSsspPerformanceCounters.h"

namespace pheet {

template <class Pheet>
class AdaptiveSssp : public Pheet::Task {
public:
	typedef AdaptiveSssp<Pheet> Self;
	typedef AdaptiveSsspStrategy<Pheet> Strategy;
	typedef AdaptiveSsspPerformanceCounters<Pheet> PerformanceCounters;

	AdaptiveSssp(SsspGraphVertex* graph, size_t size, PerformanceCounters& pc)
	:graph(graph), node(0), size(size), distance(0), k(size), successes(0), total(0), pc(pc) {}
	AdaptiveSssp(SsspGraphVertex* graph, size_t node, size_t size, size_t distance, size_t successes, size_t total, PerformanceCounters& pc)
	:graph(graph), node(node), size(size), distance(distance), k((size << 2) * successes / total), successes(successes >> 1), total(total >> 1), pc(pc) {}
	virtual ~AdaptiveSssp() {}

	virtual void operator()() {
		size_t d = graph[node].distance.load(std::memory_order_relaxed);
		if(d != distance) {
			pc.num_dead_tasks.incr();
			// Distance has already been improved in the meantime
			// No need to check again
			return;
		}
		pc.num_actual_tasks.incr();
		for(size_t i = 0; i < graph[node].num_edges; ++i) {
			++total;
			size_t new_d = d + graph[node].edges[i].weight;
			size_t target = graph[node].edges[i].target;
			size_t old_d = graph[target].distance.load(std::memory_order_relaxed);
			while(old_d > new_d) {
				if(graph[target].distance.compare_exchange_strong(old_d, new_d, std::memory_order_relaxed)) {
					++successes;
					Pheet::template
						spawn_s<Self>(
								Strategy(new_d, graph[target].distance.load(std::memory_order_relaxed), k),
								graph, target, size, new_d, successes, total, pc);
					break;
				}
			}
		}
	}

	static void print_name() {
		std::cout << name;
	}

	static char const name[];
private:
	SsspGraphVertex* graph;
	size_t node;
	size_t size;
	size_t distance;
	size_t k;
	size_t successes;
	size_t total;
	PerformanceCounters pc;
};

template <class Pheet>
char const AdaptiveSssp<Pheet>::name[] = "Adaptive Sssp";

} /* namespace pheet */
#endif /* ADAPTIVESSSP_H_ */
