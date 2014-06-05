/*
 * BBGraphBipartitioningFREELogic.h
 *
 *	Created on: Jan 13, 2012
 *			Author: Martin Wimmer, JLT
 *		 License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGFREELOGIC_H_
#define BBGRAPHBIPARTITIONINGFREELOGIC_H_

#include "pheet/pheet.h"
#include "../graph_helpers.h"

#include <string.h>
#include <algorithm>

namespace pheet {

template <class Pheet, class SubProblem>
class BBGraphBipartitioningFREELogic {
public:
	typedef BBGraphBipartitioningFREELogic<Pheet, SubProblem> Self;
	typedef typename SubProblem::Set Set;

	BBGraphBipartitioningFREELogic(SubProblem* sub_problem);
	BBGraphBipartitioningFREELogic(SubProblem* sub_problem, Self const& other);
	~BBGraphBipartitioningFREELogic();

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
	size_t get_lowdeg_lower();
	size_t cc_w();

	bool no_edges();
	void assign_deltabound();

	bool can_complete();
	void complete_solution();

	static void print_name();
private:
	SubProblem* sub_problem;

	size_t cut;
	size_t lb;
	size_t nv;
	size_t ub;
	size_t est;
	size_t contrib_sum;
	size_t lb_ub_contrib;

	size_t* weights[2];
	size_t* contributions;
	size_t* free_edges; // JLT - number of free edges (into sets[2]) for each node
	size_t* scanned[2];
	size_t* seen[2];
	size_t* fweight[2];
	size_t deg0; // number of free nodes with no edges
	size_t max_free; // largest degree of free node (approx)
	
	size_t* data;
};

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::BBGraphBipartitioningFREELogic(SubProblem* sub_problem)
	: sub_problem(sub_problem), cut(0), lb(0), nv(0), contrib_sum(0)
{
	auto numElems = sub_problem->size;
	data = new size_t[numElems * 10];
	size_t* p = data;
	
	// zero out the memory for weights, scanned, seen and fweight
	memset(p, 0, sizeof(size_t) * numElems * 8);
	weights[0] = p; p += numElems;
	weights[1] = p; p += numElems;
	scanned[0] = p; p += numElems;
	scanned[1] = p; p += numElems;
	seen[0] = p; p += numElems;
	seen[1] = p; p += numElems;
	fweight[0] = p; p += numElems;
	fweight[1] = p; p += numElems;
	
	contributions = p; p += numElems;
	free_edges = p; p += numElems;
	pheet_assert(p == data + numElems * 10);

	deg0 = 0;
	max_free = 0;
	size_t best_contrib = 0;
	size_t best_contrib_i = 0;
	for(size_t i = 0; i < sub_problem->size; ++i)
	{
		contributions[i] = 0;
		for(size_t j = 0; j < sub_problem->graph[i].num_edges; ++j)
			contributions[i] += sub_problem->graph[i].edges[j].weight;
		contrib_sum += contributions[i];

		if(contributions[i] > best_contrib)
		{
			best_contrib = contributions[i];
			best_contrib_i = i;
		}
		free_edges[i] = sub_problem->graph[i].num_edges; // all edges free
		if (free_edges[i]>max_free)
			max_free = free_edges[i];
		else if (free_edges[i]==0)
			deg0++;
	}
	nv = best_contrib_i;
	contrib_sum >>= 1;
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::BBGraphBipartitioningFREELogic(SubProblem* sub_problem, Self const& other)
	: sub_problem(sub_problem), cut(other.cut), lb(other.lb), nv(other.nv), est(other.est), contrib_sum(other.contrib_sum), deg0(other.deg0), max_free(other.max_free)
{
	auto numElems = sub_problem->size;
	data = new size_t[numElems * 10];
	size_t* p = data;

	weights[0] = p; p += numElems;
	weights[1] = p; p += numElems;
	scanned[0] = p; p += numElems;
	scanned[1] = p; p += numElems;
	seen[0] = p; p += numElems;
	seen[1] = p; p += numElems;
	fweight[0] = p; p += numElems;
	fweight[1] = p; p += numElems;

	contributions = p; p += numElems;
	free_edges = p; p += numElems;
	pheet_assert(p == data + numElems * 10);
	
	memcpy(data, other.data, sizeof(size_t) * numElems * 10); // copy all data
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::~BBGraphBipartitioningFREELogic()
{
	delete[] data;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::init_root()
{}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_next_vertex()
{
	pheet_assert(sub_problem->sets[2].test(nv));
	return nv;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_cut()
{
	return cut;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_lower_bound()
{
	return cut + lb;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_lowdeg_lower()
{
	size_t subrem[2];
	subrem[0] = sub_problem->k-sub_problem->sets[0].count();
	subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

	size_t largest_subset = (subrem[0] > subrem[1]) ? subrem[0] : subrem[1];

	if (max_free<largest_subset)
		return cut + lb; else return 0;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_estimate()
{
	pheet_assert(est + contrib_sum >= lb);
	return get_cut() + est + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_upper_bound()
{
	return get_cut() + ub + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_minnode(uint8_t set)
{
	// only one free
	Set bitset = sub_problem->sets[2];
	size_t w = bitset._Find_first();
	
	size_t mincut, sumcut = 0;
	size_t v = bitset._Find_next(w);
	while(v != bitset.size())
	{
		sumcut += weights[set][v];
		v = bitset._Find_next(v);
	}
	mincut = sumcut+weights[set^1][w]+fweight[set][w];

	v = bitset._Find_first();
	while (v != bitset.size())
	{
		sumcut += weights[set][v];
		v = bitset._Find_next(v);
		if (v != bitset.size())
		{
			sumcut -= weights[set][v];
			if (sumcut + weights[set^1][v] + fweight[set][v] < mincut)
			{
				mincut = sumcut + weights[set^1][v] + fweight[set][v];
				w = v;
			}
		}
	}

	return w;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::cc_w()
{
	// determine smallest weight edge in cc with more edges than missing in smallest subset
	size_t largest_w = std::numeric_limits<size_t>::max();

	size_t h, t;	// head and tail of queue
	ptrdiff_t c = 0; // component number
	size_t cs;		// component size
	// Switched length of array from nf to SubProblem::max_size to have a const expression which
	// is required by the C++ standard
	size_t queue[SubProblem::max_size];
	ptrdiff_t cc[SubProblem::max_size];

	size_t subrem[2];
	subrem[0] = sub_problem->k-sub_problem->sets[0].count();
	subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

	size_t largest_subset = (subrem[0] > subrem[1]) ? subrem[0] : subrem[1];

	size_t v, v0 = sub_problem->sets[2]._Find_first();
	
	size_t minw;

	Set bitset = sub_problem->sets[2];
	v = v0;
	while (v != bitset.size())
	{
		cc[v] = -1;
		v = bitset._Find_next(v);
	}

	h = 0; t = 0;
	v = v0;
	while (v != bitset.size())
	{
		if (cc[v]==-1)
		{
			// new component
			cc[v] = c;
			queue[t++] = v;
			cs = 1;
			minw = largest_w;
			while (h<t)
			{
				size_t u = queue[h++];
				const GraphVertex vertex = sub_problem->graph[u];
				for(size_t i = 0; i < vertex.num_edges; ++i)
				{
					GraphEdge edge = vertex.edges[i];
					size_t w = edge.target;
					if (bitset.test(w))
					{
						if (edge.weight < minw)
							minw = edge.weight;
						if (cc[w]==-1)
						{
							cc[w] = c;
							cs++; // could break already here
							queue[t++] = w;
						}
						else
							pheet_assert(cc[w]==c);
					}
				}
			}

			if (cs>largest_subset)
			{
				//std::cout<<minw<<':'<<cs<<'\n';
				return minw;
			}

			c++; // next component
		}
		v = bitset._Find_next(v);
	}

	return 0;
}

template <class Pheet, class SubProblem>
bool BBGraphBipartitioningFREELogic<Pheet, SubProblem>::no_edges()
{
	return (deg0 == sub_problem->sets[2].count());
 }

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::update(uint8_t set, size_t pos)
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
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::update_data(uint8_t set, size_t pos)
{
	cut += weights[set ^ 1][pos];

	size_t f, j;
	size_t subrem[2];
	subrem[0] = sub_problem->k-sub_problem->sets[0].count();
	subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

	Set bitset = sub_problem->sets[2];
	pheet_assert(subrem[0] + subrem[1] == bitset.count());

	if (free_edges[pos]==0)
		deg0--;

	const GraphVertex* graph = sub_problem->graph;
	const GraphVertex vertex = graph[pos];
	for(size_t i = 0; i < vertex.num_edges; ++i)
	{
		GraphEdge current_edge = vertex.edges[i];
		size_t v = current_edge.target;
		GraphEdge* other_edges = graph[v].edges;
		weights[set][v] += current_edge.weight;

		if (bitset.test(v))
		{
			pheet_assert(contributions[v] >= current_edge.weight);

			contributions[v] -= current_edge.weight;
			contrib_sum -= current_edge.weight;

			if (--free_edges[v]==0)
				deg0++;

			for (size_t s = 0; s < 2; ++s)
			{
				if (current_edge.reverse < scanned[s][v])
				{
					pheet_assert(fweight[s][v] >= current_edge.weight);

					fweight[s][v] -= current_edge.weight;
					seen[s][v]--;
				}
				else
				{
					f = seen[s][v];
					j = scanned[s][v];
					while (f > 0 && f + subrem[s] > free_edges[v]) // note >
					{
						j--;
						if (bitset.test(other_edges[j].target))
						{
							fweight[s][v] -= other_edges[j].weight;
							f--;
						}
					}
					seen[s][v] = f;
					scanned[s][v] = j;
				}
			}
		}
	}

	//if (deg0==bitset.count()) std::cout<<'*';

	size_t nf =  bitset.count();
	// Switched length of array from nf to SubProblem::max_size to have a const expression which
	// is required by the C++ standard
	ptrdiff_t delta[SubProblem::max_size];

	size_t di = 0;
	lb = 0;
	est = 0;
	ub = 0;
	lb_ub_contrib = 0;
	size_t v, v0 = bitset._Find_first();
	nv = v0;
	size_t best_contrib = 0;

	v = v0;
	while (v != bitset.size())
	{
		size_t weight0 = weights[0][v];
		size_t weight1 = weights[1][v];
		ptrdiff_t d = (ptrdiff_t)weight1 - (ptrdiff_t)weight0;
		if (std::abs(d) + contributions[v] >= best_contrib)
		{
			best_contrib = std::abs(d) + contributions[v];
			nv = v;
		}

		delta[di++] = d;
		lb += std::min(weight0, weight1);
		est += std::min(weight0, weight1);
		ub += std::max(weight0, weight1);

		v = bitset._Find_next(v);
	}
	std::sort(delta, delta + di);

	//size_t pivot = (/*sub_problem->size - */sub_problem->k) - sub_problem->sets[0].count();
	size_t pivot = subrem[0];
	if(pivot < di && delta[pivot] < 0)
	{
		auto d = delta[pivot];
		do
		{
			lb += -d;
			est += -d;
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
			est += delta[pivot];
			ub -= delta[pivot];
		} while(pivot > 0 && delta[pivot - 1] > 0);
	}

	// Switched length of array from nf to SubProblem::max_size to have a const expression which
	// is required by the C++ standard
	ptrdiff_t gamma[SubProblem::max_size];
	size_t gi = 0;

	size_t ss = (subrem[0] < subrem[1]) ? 0 : 1;
	size_t fw = 0;

	v = v0;
	if (max_free>=subrem[ss]) // trigger
	{
		max_free = 0;
		while (v != bitset.size())
		{
			const GraphVertex vertex = graph[v];
			pheet_assert(free_edges[v]<nf);

			if (free_edges[v] > max_free)
				max_free = free_edges[v];

			for (size_t s=0; s<2; s++)
			{
				if (subrem[s] > 0) // JLT: update should only be called when this is true
				{
					f = seen[s][v];
					j = scanned[s][v];
					//comment in next two lines to deactivate incremental computation
					//f = 0; j = 0;
					//fweight[s][v] = 0;
					while (f + subrem[s] <= free_edges[v]) // note <=
					{
						if (bitset.test(vertex.edges[j].target))
						{
							fweight[s][v] += vertex.edges[j].weight;
							f++;
						}
						j++;
					}
					seen[s][v] = f;
					scanned[s][v] = j;
				}
			}

			fw += fweight[1-ss][v];
			size_t g = fweight[ss][v]-fweight[1-ss][v];
			pheet_assert(g>=0);
			if (g>0)
				gamma[gi++] = g;
		//if (g>0) std::cout<<g<<'#'<<'\n';

			v = bitset._Find_next(v);
		}

		if (nf-gi<subrem[ss])
		{
			std::sort(gamma,gamma+gi);
			for (size_t i=0; i<subrem[ss]-(nf-gi); i++)
				fw += gamma[i];
		}
		fw >>= 1;
		lb += fw;
		est += fw;
	}

	if (max_free<subrem[1-ss])
	{
		if (cut+lb<=sub_problem->get_global_upper_bound()&&cut+lb+1000>sub_problem->get_global_upper_bound())
		{
			// How do I transform this?
			auto ccw = +cc_w();
			lb += ccw;
			est += ccw;
/*			if (get_lowdeg_lower()+cc_w(1000)>=sub_problem->get_global_upper_bound())
			{
				return; // actually irrelevant
			}*/
			//std::cout<<'#';
			// trigger
			// cc_w should go here
		}
	}

	//est += fw;
	// Is this correct (MW?)
	ub -= fw;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::bulk_update(uint8_t set, Set positions)
{
	size_t v = positions._Find_first();
	while(v != positions.size())
	{
		//update(set, v); // JLT: can be done cheaper
		sub_problem->sets[2].set(v, false);
		sub_problem->sets[set].set(v);

		cut += weights[set ^ 1][v];
			
		for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i)
		{
			size_t w = sub_problem->graph[v].edges[i].target;
			weights[set][w] += sub_problem->graph[v].edges[i].weight;
		}
		v = positions._Find_next(v);
	}

	lb = 0;
	est = 0;
	ub = 0;
	lb_ub_contrib = 0;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::assign_deltabound()
{
	size_t subrem[2];
	subrem[0] = sub_problem->k-sub_problem->sets[0].count();
	subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

//	size_t nf =	sub_problem->sets[2].count();
	// Switched length of array from nf to SubProblem::max_size to have a const expression which
	// is required by the C++ standard
	VertexDelta delta[SubProblem::max_size];

	size_t di = 0;
	size_t v = sub_problem->sets[2]._Find_first();
	while(v != sub_problem->sets[2].size())
	{
		delta[di].delta = (ptrdiff_t)weights[1][v] - (ptrdiff_t)weights[0][v];
		delta[di].target = v;
		di++;

		v = sub_problem->sets[2]._Find_next(v);
	}
	std::sort(delta, delta + di, delta_compare());

	size_t i;
	// assign to set 0
	for (i = 0; i < subrem[0]; ++i)
	{
		v = delta[i].target;
		sub_problem->sets[2].set(v, false);
		sub_problem->sets[0].set(v);

		cut += weights[1][v];
		
		// not needed
		for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i)
		{
			size_t w = sub_problem->graph[v].edges[i].target;
			weights[0][w] += sub_problem->graph[v].edges[i].weight;
			pheet_assert(!sub_problem->sets[2].test(w));
		}
	}

	// assign to set 1
	for (; i < di; ++i)
	{
		v = delta[i].target;
		sub_problem->sets[2].set(v, false);
		sub_problem->sets[1].set(v);

		cut += weights[0][v];
		
		// not needed
		for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i)
		{
			size_t w = sub_problem->graph[v].edges[i].target;
			weights[1][w] += sub_problem->graph[v].edges[i].weight;
			pheet_assert(!sub_problem->sets[2].test(w));
		}
	}
}

template <class Pheet, class SubProblem>
bool BBGraphBipartitioningFREELogic<Pheet, SubProblem>::can_complete()
{
	return ((sub_problem->sets[0].count() == sub_problem->k-1) ||
			(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1)) ||
			no_edges();
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::complete_solution()
{
	size_t s;
	if(sub_problem->sets[0].count() == sub_problem->k-1)
		s = 0;
	else if(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1)
		s = 1;
	else
	{
		pheet_assert(no_edges());

		assign_deltabound();
		return;
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
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::print_name()
{
	std::cout << "FREELogic";
}

}

#endif /* BBGRAPHBIPARTITIONINGFREELOGIC_H_ */
