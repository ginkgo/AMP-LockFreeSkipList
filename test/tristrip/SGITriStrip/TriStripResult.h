/*
 * TriStripResult.h
 *
 *  Created on: Jul 19, 2012
 *      Author: cederman
 */

#ifndef TRISTRIPRESULT_H_
#define TRISTRIPRESULT_H_

//#include <pheet/pheet.h>
#include "pheet/pheet.h"
#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/Events/EventsList.h"

namespace pheet
{

template <class Pheet>
class TriStripResult
{
 public:
  //    BasicPerformanceCounter<Pheet, true> tristripcount;
  //BasicPerformanceCounter<Pheet, true> nodecount;

  //	SumReducer<Pheet, size_t> tristripcount;
  //	SumReducer<Pheet, size_t> nodecount;

public:
	TriStripResult() {};
	//	TriStripResult(TriStripResult<Pheet>& other):tristripcount(other.tristripcount),nodecount(other.nodecount) {}
	//TriStripResult(TriStripResult<Pheet>&& other):tristripcount(other.tristripcount),nodecount(other.nodecount) {};

	//	void addstrip(std::vector<GraphNode*> strip)
	//	{
	  //	nodecount.add(strip.size());
		//		printf("%d\n",strip.size());
		// Should store the strip, but for now only count it
	  //	tristripcount.incr();
	//	}

	//size_t getNodeCount()
	  //	{
	  // return 0;
	  //	return nodecount.get_sum();
	  //}

	//	size_t getCount()
	//	{
	// return 0;
	  //	return tristripcount.get_sum();
	//	}
	//	BasicPerformanceCounter<Pheet, true> tristripcount;
};


}

#endif /* TRISTRIPRESULT_H_ */
