/*
 * SsspInitTask.h
 *
 *  Created on: Mar 13, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSPINITTASK_H_
#define SSSPINITTASK_H_

#include <vector>

#include "sssp_graph_helpers.h"

namespace pheet {

template <class Pheet>
class SsspInitTask : public Pheet::Task {
public:
	typedef SsspInitTask<Pheet> Self;

	SsspInitTask(SsspGraphVertex* data, std::vector<SsspGraphEdge>* edges, size_t begin, size_t end)
	: data(data),
	  edges(edges),
	  begin(begin),
	  end(end) {}
	~SsspInitTask() {}

	virtual void operator()() {
		if(begin != end) {
			pheet_assert(begin < end);
			size_t middle = begin + ((end - begin) >> 1);

			Pheet::template
				spawn<Self>(data, edges, begin, middle);
			Pheet::template
				call<Self>(data, edges, middle + 1, end);
		}
		else {
			if(edges[begin].size() > 0) {
				data[begin].edges = new SsspGraphEdge[edges[begin].size()];
				for(size_t j = 0; j < edges[begin].size(); ++j) {
					data[begin].edges[j] = edges[begin][j];
				}
			}
			else {
				data[begin].edges = nullptr;
			}
			data[begin].num_edges = edges[begin].size();
			data[begin].distance = std::numeric_limits<size_t>::max();
		}
	}

private:
	SsspGraphVertex* data;
	std::vector<SsspGraphEdge>* edges;
	size_t begin;
	size_t end;
};

} /* namespace pheet */
#endif /* SSSPINITTASK_H_ */
