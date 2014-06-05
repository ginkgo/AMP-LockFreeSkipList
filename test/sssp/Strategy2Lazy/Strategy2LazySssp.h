/*
 * Strategy2LazySssp.h
 *
 *  Created on: Oct 08, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef STRATEGY2LAZYSSSP_H_
#define STRATEGY2LAZYSSSP_H_

#include "Strategy2LazySsspStrategy.h"
#include "Strategy2LazySsspPerformanceCounters.h"
#include "Strategy2LazySsspLocalDistances.h"

namespace pheet {

template <class Pheet>
class Strategy2LazySssp : public Pheet::Task {
public:
	typedef Strategy2LazySssp<Pheet> Self;
	typedef Strategy2LazySsspStrategy<Pheet> Strategy;
	typedef Strategy2LazySsspPerformanceCounters<Pheet> PerformanceCounters;

	Strategy2LazySssp(SsspGraphVertex* graph, size_t size, PerformanceCounters& pc)
	:graph(graph), node(0), distance(0), pc(pc) {
		pc.last_non_dead_time.start_timer();
		pc.last_task_time.start_timer();
		pc.last_update_time.start_timer();
	}
	Strategy2LazySssp(SsspGraphVertex* graph, size_t node, size_t distance, PerformanceCounters& pc)
	:graph(graph), node(node), distance(distance), pc(pc) {}
	virtual ~Strategy2LazySssp() {}

	virtual void operator()() {
		pc.last_task_time.take_time();
		auto& dist = Pheet::template place_singleton<Strategy2LazySsspLocalDistances<Pheet>>();
		size_t d = graph[node].distance.load(std::memory_order_relaxed);
		if(node != 0 && (d <= distance || !graph[node].distance.compare_exchange_strong(d, distance, std::memory_order_relaxed))) {
			dist[node] = d;
			pc.num_dead_tasks.incr();
			// Distance has already been improved in the meantime
			// No need to check again
			return;
		}
		dist[node] = distance;
		d = distance;

		pc.num_actual_tasks.incr();
		pc.last_non_dead_time.take_time();

		for(size_t i = 0; i < graph[node].num_edges; ++i) {
			size_t new_d = d + graph[node].edges[i].weight;
			size_t target = graph[node].edges[i].target;

			size_t& local_d = dist[target];
			if(local_d > new_d) {
				local_d = new_d;

				size_t old_d = graph[target].distance.load(std::memory_order_relaxed);
				if(old_d > new_d) {
					Pheet::template
						spawn_s<Self>(
							Strategy(new_d, graph[target].distance),
							graph, target, new_d, pc);
				}
				else {
					local_d = old_d;
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

template <class Pheet>
char const Strategy2LazySssp<Pheet>::name[] = "Strategy2 Lazy Sssp";

} /* namespace pheet */
#endif /* STRATEGY2LAZYSSSP_H_ */
