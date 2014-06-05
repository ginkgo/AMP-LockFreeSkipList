/*
 * sssp_graph_helpers.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSP_GRAPH_HELPERS_H_
#define SSSP_GRAPH_HELPERS_H_

#include <atomic>

namespace pheet {
struct SsspGraphEdge {
	size_t target;
	size_t weight;
};

struct SsspGraphVertex {
	SsspGraphEdge* edges;
	size_t num_edges;
	std::atomic<size_t> distance;
};

}


#endif /* SSSP_GRAPH_HELPERS_H_ */
