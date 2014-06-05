/*
 * ReferenceSssp.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef REFERENCESSSP_H_
#define REFERENCESSSP_H_

#include "ReferenceSsspPerformanceCounters.h"

//#include <pheet/ds/PriorityQueue/Merge/MergeHeap.h>

namespace pheet {

struct ReferenceSsspNode {
	size_t node_id;
	size_t distance;
};

struct ReferenceSsspNodeLess {
	bool operator()(ReferenceSsspNode const& n1, ReferenceSsspNode const& n2) {
		return n1.distance > n2.distance;
	}
};

template <class Pheet>
class ReferenceSssp : public Pheet::Task {
public:
	typedef ReferenceSsspPerformanceCounters<Pheet> PerformanceCounters;

	ReferenceSssp(SsspGraphVertex* graph, size_t, PerformanceCounters& pc)
	:graph(graph), pc(pc) {}
	virtual ~ReferenceSssp() {}

	virtual void operator()() {
		typename Pheet::DS::template Heap<ReferenceSsspNode, ReferenceSsspNodeLess> heap;
//		MergeHeap<Pheet, ReferenceSsspNode, ReferenceSsspNodeLess> heap;

		ReferenceSsspNode n;
		n.distance = 0;
		n.node_id = 0;
		heap.push(n);

		while(!heap.empty()) {
			n = heap.pop();
			size_t node = n.node_id;

			size_t d = graph[node].distance.load(std::memory_order_relaxed);
			if(d != n.distance) {
				pc.num_dead_tasks.incr();
				// Distance has already been improved in the meantime
				// No need to check again
				continue;
			}
			pc.num_actual_tasks.incr();
			for(size_t i = 0; i < graph[node].num_edges; ++i) {
				size_t new_d = d + graph[node].edges[i].weight;
				size_t target = graph[node].edges[i].target;
				size_t old_d = graph[target].distance;
				if(old_d > new_d) {
					graph[target].distance.store(new_d, std::memory_order_relaxed);
					n.distance = new_d;
					n.node_id = target;
					heap.push(n);
				}
			}
		}
	}

	static void set_k(size_t) {}

	static void print_name() {
		std::cout << name;
	}

	static char const name[];
private:
	SsspGraphVertex* graph;
	PerformanceCounters pc;
};

template <class Pheet>
char const ReferenceSssp<Pheet>::name[] = "Reference Sssp";

} /* namespace pheet */
#endif /* REFERENCESSSP_H_ */
