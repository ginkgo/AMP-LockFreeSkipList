/*
 * ParallelRecursiveNQueens.h Parallel and recursive N-queens solver
 *
 *  Created on: 2011-10-11
 *      Author: Anders Gidenstam
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PARALLELRECURSIVENQUEENS_H_
#define PARALLELRECURSIVENQUEENS_H_

#include <algorithm>

#include "../../../pheet/misc/types.h"
#include "ParallelRecursiveNQueensTask.h"

namespace pheet {

template <class Pheet>
class ParallelRecursiveNQueens {
public:
  ParallelRecursiveNQueens(procs_t cpus, size_t N);
  ~ParallelRecursiveNQueens();

  int* solve();
  void print_results();
  void print_headers();

  static void print_scheduler_name();

  static const procs_t max_cpus;
  static const char name[];

private:
  size_t cpus;
  size_t N;
  typename Pheet::Environment::PerformanceCounters pc;

};


template <class Pheet>
char const ParallelRecursiveNQueens<Scheduler>::name[] = "Parallel Recursive N-Queens";

template <class Scheduler>
ParallelRecursiveNQueens<Scheduler>::ParallelRecursiveNQueens(procs_t cpus, size_t n)
  : cpus(cpus), N(n) { //, cpu_hierarchy(cpus), scheduler(&cpu_hierarchy) {

}

template <class Scheduler>
ParallelRecursiveNQueens<Scheduler>::~ParallelRecursiveNQueens() {

}

template <class Scheduler>
int* ParallelRecursiveNQueens<Scheduler>::solve() {
  typename ParallelRecursiveNQueensTask<typename Pheet::Task>::subproblem_t* problem;
  int* result = 0;

  problem = (typename ParallelRecursiveNQueensTask<typename Pheet::Task>::subproblem_t*)malloc(sizeof(typename ParallelRecursiveNQueensTask<typename Pheet::Task>::subproblem_t));

  problem->N        = N;
  problem->column   = 0;
  problem->solution = (int*)calloc(N, sizeof(int));
  problem->result   = &result;
  {typename Pheet::Environment env(cpus,pc);
  scheduler.template finish<ParallelRecursiveNQueensTask<typename Pheet::Task> >(problem);
  }
  return result;
}

template <class Scheduler>
void ParallelRecursiveNQueens<Scheduler>::print_results() {
	pc.print_values();
}

template <class Scheduler>
void ParallelRecursiveNQueens<Scheduler>::print_headers() {
	Pheet::Environment::PerformanceCounters::print_headers();
}

template <class Scheduler>
void ParallelRecursiveNQueens<Scheduler>::print_scheduler_name() {
	Pheet::Environment::print_name();
}

}

#endif /* PARALLELRECURSIVENQUEENS_H_ */
