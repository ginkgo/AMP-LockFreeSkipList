/*
 * BBGraphBipartitioningLogic.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGLOGIC_H_
#define BBGRAPHBIPARTITIONINGLOGIC_H_

#include "pheet/pheet.h"
#include "../graph_helpers.h"

#include <string.h>
#include <algorithm>

namespace pheet {

template <class Pheet, class SubProblem>
class BBGraphBipartitioningLogic {
public:
	typedef BBGraphBipartitioningLogic<Pheet, SubProblem> Self;
	typedef typename SubProblem::Set Set;

	BBGraphBipartitioningLogic(SubProblem* sub_problem);
	BBGraphBipartitioningLogic(SubProblem* sub_problem, Self const& other);
	~BBGraphBipartitioningLogic();

	void init_root();
	size_t get_next_vertex();
	size_t get_cut();
	size_t get_lower_bound();
	size_t get_estimate();
	size_t get_upper_bound();
	void update(uint8_t set, size_t pos);
	void update_data(uint8_t set, size_t pos);
	void bulk_update(uint8_t set, Set positions);

	size_t get_minnode(uint8_t set);
//	size_t get_lowdeg_lower();
//	size_t cc_w(size_t largest_w);

//	bool no_edges();
//	void assign_deltabound();

	bool can_complete();
	void complete_solution();

	static void print_name();
private:
	SubProblem* sub_problem;

	size_t cut;
	size_t lb;
	size_t nv;
	size_t ub;
	size_t contrib_sum;

	size_t* weights[2];
	size_t* contributions;
	size_t* data;
};

template <class Pheet, class SubProblem>
BBGraphBipartitioningLogic<Pheet, SubProblem>::BBGraphBipartitioningLogic(SubProblem* sub_problem)
	: sub_problem(sub_problem), cut(0), lb(0), nv(0), ub(0), contrib_sum(0)
{
	auto numElems = sub_problem->size;
	data = new size_t[numElems * 3];
	size_t* p = data;
	
	memset(p, 0, sizeof(size_t) * numElems * 2); // zero out weights
	weights[0] = p; p += numElems;
	weights[1] = p; p += numElems;
	contributions = p; p += numElems;
	pheet_assert(p == data + numElems * 3);

	size_t best_contrib = 0;
	size_t best_contrib_i = 0;
	for(size_t i = 0; i < sub_problem->size; ++i)
	{
		contributions[i] = 0;
		for(size_t j = 0; j < sub_problem->graph[i].num_edges; ++j)
		{
			contributions[i] += sub_problem->graph[i].edges[j].weight;
		}
		contrib_sum += contributions[i];
		if(contributions[i] > best_contrib)
		{
			best_contrib = contributions[i];
			best_contrib_i = i;
		}
	}
	nv = best_contrib_i;
	contrib_sum >>= 1;
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningLogic<Pheet, SubProblem>::BBGraphBipartitioningLogic(SubProblem* sub_problem, Self const& other)
	: sub_problem(sub_problem), cut(other.cut), lb(other.lb), nv(other.nv), ub(other.ub), contrib_sum(other.contrib_sum)
{
 	auto numElems = sub_problem->size;
	data = new size_t[numElems * 3];
	size_t* p = data;

	weights[0] = p; p += numElems;
	weights[1] = p; p += numElems;
	contributions = p;
	memcpy(data, other.data, sizeof(size_t) * numElems * 3);
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningLogic<Pheet, SubProblem>::~BBGraphBipartitioningLogic()
{
	delete[] data;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::init_root()
{}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_next_vertex()
{
	pheet_assert(sub_problem->sets[2].test(nv));
	return nv;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_cut()
{
	return cut;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_lower_bound()
{
	return get_cut() + lb;
}
/*
template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_lowdeg_lower() {
  return 0;
}*/

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_estimate()
{
//	return get_cut() + ((lb + ub + contrib_sum) >> 1);
	return get_cut() + lb + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_upper_bound()
{
	return get_cut() + ub + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::get_minnode(uint8_t set)
{
	// only one free

	size_t fweight;

	Set bitset = sub_problem->sets[2];
	size_t w = bitset._Find_first();
	fweight = 0;
	const GraphVertex vertex = sub_problem->graph[w];
	for (size_t i = 0; i < vertex.num_edges; ++i)
	{
		if (bitset.test(vertex.edges[i].target))
			fweight += vertex.edges[i].weight;
	}

	size_t mincut, sumcut = 0;
	size_t v = bitset._Find_next(w);
	while (v != bitset.size())
	{
		sumcut += weights[set][v];
		v = bitset._Find_next(v);
	}

	mincut = sumcut + weights[set^1][w] + fweight;

	v = bitset._Find_first();
	while (v != bitset.size())
	{
		sumcut += weights[set][v];
		v = bitset._Find_next(v);
		if (v != bitset.size())
		{
			fweight = 0;
			for (size_t i=0; i<sub_problem->graph[v].num_edges; ++i)
			{
				if (bitset.test(sub_problem->graph[v].edges[i].target))
					fweight += sub_problem->graph[v].edges[i].weight;
			}
			sumcut -= weights[set][v];
			if (sumcut + weights[set^1][v] + fweight < mincut)
			{
				mincut = sumcut+weights[set^1][v]+fweight;
				w = v;
			}
		}
	}

	return w;
}
/*
template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningLogic<Pheet, SubProblem>::cc_w(size_t largest_w) {
  return 0;
}
*/
/*
template <class Pheet, class SubProblem>
bool BBGraphBipartitioningLogic<Pheet, SubProblem>::no_edges() {
  return 0;
 }
*/
template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::update(uint8_t set, size_t pos)
{
	pheet_assert((set & 1) == set);
	pheet_assert(pos < sub_problem->size);
	pheet_assert(!sub_problem->sets[set].test(pos));
	pheet_assert(sub_problem->sets[2].test(pos));

	sub_problem->sets[2].set(pos, false);
	sub_problem->sets[set].set(pos);

	update_data(set, pos);
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::update_data(uint8_t set, size_t pos)
{
	cut += weights[set ^ 1][pos];

	Set bitset = sub_problem->sets[2];
	const GraphVertex vertex = sub_problem->graph[pos];
	for(size_t i = 0; i < vertex.num_edges; ++i)
	{
		const GraphEdge edge = vertex.edges[i];
		weights[set][edge.target] += edge.weight;
		if (bitset.test(edge.target))
		{
			pheet_assert(contributions[edge.target] >= edge.weight);
			contributions[edge.target] -= edge.weight;
			contrib_sum -= edge.weight;
		}
	}

//	size_t nf =  bitset.count();
	// Switched length of array from nf to SubProblem::max_size to have a const expression which
	// is required by the C++ standard
	ptrdiff_t delta[SubProblem::max_size];
	//ptrdiff_t* delta = new ptrdiff_t[bitset.count()];

	size_t di = 0;
	lb = 0;
	ub = 0;
	size_t current_bit = bitset._Find_first();
	nv = current_bit;
	size_t best_contrib = 0;
	while (current_bit != bitset.size())
	{
		size_t weight0 = weights[0][current_bit];
		size_t weight1 = weights[1][current_bit];
		ptrdiff_t d = (ptrdiff_t)weight1 - (ptrdiff_t)weight0;
		if (std::abs(d) + contributions[current_bit] >= best_contrib)
		{
			best_contrib = std::abs(d) + contributions[current_bit];
			nv = current_bit;
		}
		delta[di++] = d;
		lb += std::min(weight0, weight1);
		ub += std::max(weight0, weight1);
		current_bit = bitset._Find_next(current_bit);
	}
	std::sort(delta, delta + di);

	size_t pivot = (/*sub_problem->size - */sub_problem->k) - sub_problem->sets[0].count();
	if (pivot < di && delta[pivot] < 0)
	{
		auto d = delta[pivot];
		do
		{
			lb += -d;
			ub -= -d;
			++pivot;
			d = delta[pivot];
		} while(pivot < di && d < 0);
	}
	else if(pivot > 0 && delta[pivot - 1] > 0)
	{
		do
		{
			--pivot;
			lb += delta[pivot];
			ub -= delta[pivot];
		} while(pivot > 0 && delta[pivot - 1] > 0);
	}

	//delete[] delta;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::bulk_update(uint8_t set, Set positions)
{
	size_t current_bit = positions._Find_first();
	while (current_bit != positions.size())
	{
		update_data(set, current_bit);
		current_bit = positions._Find_next(current_bit);
	}
}

template <class Pheet, class SubProblem>
bool BBGraphBipartitioningLogic<Pheet, SubProblem>::can_complete()
{
	return ((sub_problem->sets[0].count() == sub_problem->k-1) ||
			(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1));
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::complete_solution()
{
	size_t s;
	if(sub_problem->sets[0].count() == sub_problem->k-1)
		s = 0;
	else {
		pheet_assert(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1);
		s = 1;
	}

	//std::cout<<'#';

	size_t v = get_minnode(s);

	update(s,v);

	sub_problem->sets[1-s] |= sub_problem->sets[2];
	Set tmp = sub_problem->sets[2];
	sub_problem->sets[2].reset();
	bulk_update(1-s, tmp);
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningLogic<Pheet, SubProblem>::print_name()
{
	std::cout << "Logic";
}

}

#endif /* BBGRAPHBIPARTITIONINGLOGIC_H_ */
