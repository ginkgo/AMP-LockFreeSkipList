/*
* TriStripRun.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef TriStripRUN_H_
#define TriStripRUN_H_

#include <pheet/pheet.h>
#include "TriStripResult.h"

#include "TriStripTasks.h"
#include <iostream>
#include <fstream>
//#include "TriStripPerformanceCounters.h"
#include "../GraphDual.h"
#include "TriStripResult.h"

namespace pheet {



template <class Pheet>
class TriStripRun {
public:
  TriStripRun(procs_t cpus, GraphDual& graph, bool withstrat);
  ~TriStripRun();

  void run();

  void print_results();
  void print_headers();

  static void print_name();
  static void print_scheduler_name();

  static char const name[];


  size_t getTriStripCount();
  size_t getNodeTriStripCount();


private:
	unsigned int cpus;
	GraphDual& graph;
	bool withstrat;
	TriStripResult<Pheet> result;
	typename Pheet::Environment::PerformanceCounters pc;
	TriStripPerformanceCounters<Pheet> ppc;
};


template <class Scheduler>
char const TriStripRun<Scheduler>::name[] = "TriStrip";

template <class Pheet>
size_t TriStripRun<Pheet>::getNodeTriStripCount()
{
  //	return result.getNodeCount();
  return ppc.getNodeCount();
}




template <class Pheet>
size_t TriStripRun<Pheet>::getTriStripCount()
{
  //	return result.getCount();
  return ppc.getCount();
}

template <class Pheet>
  TriStripRun<Pheet>::TriStripRun(procs_t cpus, GraphDual& graph, bool withstrat):
 cpus(cpus), graph(graph),withstrat(withstrat) {

}

template <class Pheet>
TriStripRun<Pheet>::~TriStripRun() {

}

template <class Pheet>
void TriStripRun<Pheet>::run() {
	// Start here
//	typename Pheet::Environment env(cpus,pc);
	if(withstrat)
	  Pheet::template finish<TriStripStartTask<Pheet,true> >(graph, result, ppc);
	else
	  Pheet::template finish<TriStripStartTask<Pheet,false> >(graph, result, ppc);
}


template <class Pheet>
void TriStripRun<Pheet>::print_results() {
//	pc.print_values();
	ppc.print_values();
}

template <class Pheet>
void TriStripRun<Pheet>::print_headers() {
//	Pheet::Environment::PerformanceCounters::print_headers();
	ppc.print_headers();
}

template <class Pheet>
void TriStripRun<Pheet>::print_name() {
	std::cout << name;
}

template <class Pheet>
void TriStripRun<Pheet>::print_scheduler_name() {
	Pheet::Environment::print_name();
}
}

#endif /* TriStripRUN_H_ */
