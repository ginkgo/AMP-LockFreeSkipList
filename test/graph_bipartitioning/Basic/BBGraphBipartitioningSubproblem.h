/*
 * BBGraphBipartitioningSubproblem.h
 *
 *  Created on: Dec 1, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGSUBPROBLEM_H_
#define BBGRAPHBIPARTITIONINGSUBPROBLEM_H_

#include "pheet/pheet.h"
#include "../graph_helpers.h"
#include "../FastBitset.h"
#include "pheet/primitives/Reducer/Max/MaxReducer.h"

#include "BBGraphBipartitioningSubproblemPerformanceCounters.h"

#include <bitset>

namespace pheet {

// Needed to resolve circular template dependency problem in gcc
template <class Pheet, size_t MaxSize = 64>
class BBGraphBipartitioningSubproblemBase {
protected:
	BBGraphBipartitioningSubproblemBase(GraphVertex const* graph, size_t size, size_t k, std::atomic<size_t>* upper_bound)
	: graph(graph), size(size), k(k), upper_bound(upper_bound) {}

public:
	static size_t const max_size = MaxSize;

	//typedef std::bitset<MaxSize> Set;
	typedef FastBitset<MaxSize> Set;

	GraphVertex const* const graph;
//size_t const lw; // largest weight edge (JLT: more dynamic approx. perhaps better)
	size_t const size;
	size_t const k;

	std::atomic<size_t>* upper_bound;

	Set sets[3];

	size_t get_global_upper_bound() {
		return this->upper_bound->load(std::memory_order_relaxed);
	}
// JLT would like
// uint8 assigned[size] = 0/1/2 - where is vertex assigned?
// free[size] = [0,...,size-1]; - list of free vertices
// subsetsize[3]; current size of subsets, subsetsize[2] is number of free vertices
};

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize = 64>
class BBGraphBipartitioningSubproblem : public BBGraphBipartitioningSubproblemBase<Pheet, MaxSize> {
public:
	typedef std::bitset<MaxSize> Set;
	typedef BBGraphBipartitioningSubproblemPerformanceCounters<Pheet> PerformanceCounters;
	typedef BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize> Self;
	typedef BBGraphBipartitioningSubproblemBase<Pheet, MaxSize> Base;
	typedef GraphBipartitioningSolution<MaxSize> Solution;
	typedef MaxReducer<Pheet, Solution> SolutionReducer;
	typedef LogicT<Pheet, Base> Logic;

	BBGraphBipartitioningSubproblem(GraphVertex const* graph, size_t size, size_t k, std::atomic<size_t>* upper_bound);
	BBGraphBipartitioningSubproblem(Self const& other);
	~BBGraphBipartitioningSubproblem();

	Self* split(PerformanceCounters& pc);
	bool can_complete(PerformanceCounters&) {
		return logic.can_complete();
	}
	void complete_solution(PerformanceCounters&) {
		logic.complete_solution();
	}
//	bool is_solution();
	void update(uint8_t set, size_t pos);
	void update_solution(SolutionReducer& best, PerformanceCounters& pc);
	size_t get_lower_bound();
	size_t get_old_estimate() { return old_estimate; }
	size_t get_old_lower_bound() { return old_lb; }
	size_t get_estimate();
	size_t get_upper_bound();

//	size_t get_lowdeg_lower();
//	size_t cc_w(size_t largest_w);

private:
	Logic logic;
	size_t old_lb;
	size_t old_estimate;
};

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::BBGraphBipartitioningSubproblem(GraphVertex const* graph, size_t size, size_t k, std::atomic<size_t>* upper_bound)
: Base(graph, size, k, upper_bound), logic(this), old_lb(0), old_estimate(0) {
	for(size_t i = 0; i < size; ++i) {
		this->sets[2].set(i);
	}
	if(this->k == this->size - this->k) {
		// If both groups have the same size we can already fill in the first vertex
		size_t nv = logic.get_next_vertex();
		update(0, nv);
	}
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::BBGraphBipartitioningSubproblem(Self const& other)
: Base(other.graph, other.size, other.k, other.upper_bound), logic(this, other.logic) {
	for(size_t i = 0; i < 3; ++i) {
		this->sets[i] = other.sets[i];
	}
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::~BBGraphBipartitioningSubproblem() {

}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>* BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::split(PerformanceCounters& pc) {
	pc.num_allocated_subproblems.incr();
	pc.memory_allocation_time.start_timer();
	Self* other = new Self(*this);
	pc.memory_allocation_time.stop_timer();

	size_t nv = logic.get_next_vertex();
	update(0, nv);
	other->update(1, nv);

	return other;
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
void BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::update(uint8_t set, size_t pos) {
/*	pheet_assert((set & 1) == set);
	pheet_assert(pos < this->size);
	pheet_assert(!this->sets[set].test(pos));
	pheet_assert(this->sets[2].test(pos));

	this->sets[2].set(pos, false);
	this->sets[set].set(pos);
*/
//	uint8_t other_set = set ^ 1;

	old_lb = logic.get_lower_bound();
	old_estimate = logic.get_estimate();
	logic.update(set, pos); // JLT - sometimes not good
}
/*
template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
bool BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::is_solution() {
	return (this->sets[0].count() == this->k) || (this->sets[1].count() == (this->size - this->k));
}
*/
template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
void BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::update_solution(MaxReducer<Pheet, GraphBipartitioningSolution<MaxSize> >& best, PerformanceCounters& pc) {
/*	if(this->sets[0].count() == this->k) {
		this->sets[1] |= this->sets[2];
		Set tmp = this->sets[2];
		this->sets[2].reset();
		logic.bulk_update(1, tmp);
	}
	else {
		pheet_assert(this->sets[1].count() == (this->size - this->k));
		this->sets[0] |= this->sets[2];
		Set tmp = this->sets[2];
		this->sets[2].reset();
		logic.bulk_update(0, tmp);
	}*/
	pheet_assert(this->sets[0].count() == this->k && this->sets[1].count() == (this->size - this->k));

	size_t cut = logic.get_cut();
	size_t old_ub = this->get_global_upper_bound();

	while(cut < old_ub) {
		if(this->upper_bound->compare_exchange_weak(old_ub, cut, std::memory_order_relaxed)) {
			pc.last_update_time.take_time();
			pc.num_upper_bound_changes.incr();

			pc.events.add(cut);

			GraphBipartitioningSolution<MaxSize> my_best;
			my_best.weight = cut;
			my_best.sets[0] = this->sets[0];
			my_best.sets[1] = this->sets[1];
			best.add_value(my_best);
		}
	}
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
size_t BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::get_lower_bound() {
	return logic.get_lower_bound();
}
/*
template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
size_t BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::get_lowdeg_lower() {
	return logic.get_lowdeg_lower();
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
size_t BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::cc_w(size_t largest_w) {
	return logic.cc_w(largest_w);
}
*/
template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
size_t BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::get_estimate() {
	return logic.get_estimate();
}

template <class Pheet, template <class P, class SP> class LogicT, size_t MaxSize>
size_t BBGraphBipartitioningSubproblem<Pheet, LogicT, MaxSize>::get_upper_bound() {
	return logic.get_upper_bound();
}

}

#endif /* BBGRAPHBIPARTITIONINGSUBPROBLEM_H_ */
