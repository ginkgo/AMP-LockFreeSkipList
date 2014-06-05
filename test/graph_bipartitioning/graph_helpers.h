/*
 * graph_helpers.h
 *
 *  Created on: 07.09.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef GRAPH_HELPERS_H_
#define GRAPH_HELPERS_H_

#include "../init.h"
#include "FastBitset.h"
#include "pheet/misc/types.h"
#include "pheet/primitives/Reducer/Max/MaxReducer.h"

#include <bitset>

namespace pheet {

struct GraphEdge {
	size_t target;
	size_t weight;
        size_t reverse;
};

struct GraphVertex {
        GraphEdge* edges; // JLT: why not vector<GrahpEdge>?
	size_t num_edges;
};

class edgeweight_compare {
 public:
  int operator()(const GraphEdge &e1, const GraphEdge &e2) const
  { 
    return (e1.weight<e2.weight);
  }
};

struct VertexDelta {
	size_t target;
	ptrdiff_t delta;
};
 
class delta_compare {
 public:
  int operator()(const VertexDelta &d1, const VertexDelta &d2) const
  { 
    return (d1.delta<d2.delta);
  }
};

template <size_t MAX_SIZE = 64>
struct GraphBipartitioningSolution {
	GraphBipartitioningSolution();

	GraphBipartitioningSolution<MAX_SIZE>& operator=(GraphBipartitioningSolution<MAX_SIZE> const& other);
	GraphBipartitioningSolution<MAX_SIZE>& operator=(GraphBipartitioningSolution<MAX_SIZE>& other);

	size_t weight;
	//std::bitset<MAX_SIZE> sets[2];
	FastBitset<MAX_SIZE> sets[2];
};

template <size_t MAX_SIZE>
GraphBipartitioningSolution<MAX_SIZE>::GraphBipartitioningSolution() {
	weight = 0;
}

template <size_t MAX_SIZE>
GraphBipartitioningSolution<MAX_SIZE>& GraphBipartitioningSolution<MAX_SIZE>::operator=(GraphBipartitioningSolution<MAX_SIZE> const& other) {
	weight = other.weight;
	sets[0] = other.sets[0];
	sets[1] = other.sets[1];
	return *this;
}

template <size_t MAX_SIZE>
GraphBipartitioningSolution<MAX_SIZE>& GraphBipartitioningSolution<MAX_SIZE>::operator=(GraphBipartitioningSolution<MAX_SIZE>& other) {
	weight = other.weight;
	sets[0] = other.sets[0];
	sets[1] = other.sets[1];
	return *this;
}

template <size_t MAX_SIZE>
struct MaxOperation<GraphBipartitioningSolution<MAX_SIZE> > {
	GraphBipartitioningSolution<MAX_SIZE>& operator()(GraphBipartitioningSolution<MAX_SIZE>& x, GraphBipartitioningSolution<MAX_SIZE>& y);

	GraphBipartitioningSolution<MAX_SIZE> get_identity();
};

template <size_t MAX_SIZE>
GraphBipartitioningSolution<MAX_SIZE>& MaxOperation<GraphBipartitioningSolution<MAX_SIZE> >::operator()(GraphBipartitioningSolution<MAX_SIZE>& x, GraphBipartitioningSolution<MAX_SIZE>& y) {
	if(x.weight <= y.weight)
		return x;
	return y;
}

template <size_t MAX_SIZE>
GraphBipartitioningSolution<MAX_SIZE> MaxOperation<GraphBipartitioningSolution<MAX_SIZE> >::get_identity() {
	GraphBipartitioningSolution<MAX_SIZE> ret;
	ret.weight = std::numeric_limits<size_t>::max();
	return ret;
}

// note: data must have space for length + 1 elements 
template <typename T>
static inline void insert(T elem, T *data, int length)
{
	T* p = data + length - 1;
	while ((size_t)p >= (size_t)data && *p > elem)
	{
		p[1] = p[0];
		--p;
	}
	p[1] = elem;
}

}

#endif /* GRAPH_HELPERS_H_ */
