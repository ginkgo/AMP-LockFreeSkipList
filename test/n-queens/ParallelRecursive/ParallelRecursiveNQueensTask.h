/*
 * ParallelRecursiveNQueens.cpp Parallel and recursive N-queens solver
 *
 *  Created on: 2011-10-11
 *      Author: Anders Gidenstam
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PARALLELRECURSIVENQUEENSTASK_H_
#define PARALLELRECURSIVENQUEENSTASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/atomics.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace pheet {

template <class Pheet>
class ParallelRecursiveNQueensTask : public Pheet::Task {
public:

  typedef struct {
    int    N;
    int    column;
    int*   solution;
    int**  result;
  } subproblem_t;

  ParallelRecursiveNQueensTask(subproblem_t* p);
  virtual ~ParallelRecursiveNQueensTask();
  virtual void operator()();
//  virtual void operator()(typename Task::Scheduler::TaskExecutionContext &tec);

private:
  subproblem_t* problem;
};


template <class Pheet>
ParallelRecursiveNQueensTask<Pheet>::ParallelRecursiveNQueensTask(typename ParallelRecursiveNQueensTask<Pheet>::subproblem_t* p)
  : problem(p) {

}

template <class Pheet>
ParallelRecursiveNQueensTask<Pheet>::~ParallelRecursiveNQueensTask() {

}

template <class Pheet>
void ParallelRecursiveNQueensTask<Pheet>::operator()() {
//(typename Task::Scheduler::TaskExecutionContext &tec) {

	typename Pheet::Finish wait_for_all_children();
  // There is no need for the task to wait for its children.

  for (int row = 0; row < problem->N; row++) {
    bool allowed = true;

    if (*(problem->result) != 0) {
      /* A solution has been found elsewhere. */
      break;
    }

    // Check if this partial solution is feasible.
    for (int c = 0; c < problem->column; c++) {
      if ((problem->solution[c] == row) ||
	  ((problem->column - c) == (problem->solution[c] - row)) ||
	  ((problem->column - c) == (row - problem->solution[c]))) {
	/* Disallowed row. */
	allowed = 0;
	break;
      }
    }

    if (allowed) {
      problem->solution[problem->column] = row;

      if (problem->column == problem->N - 1) {
	/* The solution is complete. */
	if (PTR_CAS(problem->result, 0, problem->solution)) {
	  problem->solution = 0;
	}
	break;
      } else if (0 /*p->N - p->column <= CUT_OFF*/) {
	// FIXME: Insert cut-off here if desired.
	/*
        if (solve_sequential(problem->N, problem->column + 1, problem->solution)) {
          if (PTR_CAS(problem->result, 0, problem->solution)) {
            problem->solution = 0;
          }
          break;
        } else {
          problem->solution[problem->column] = -1;
        }
	*/
      } else {
	/* Spawn a sub task. */
	subproblem_t* sp = (subproblem_t*)malloc(sizeof(subproblem_t));
	memcpy(sp, problem, sizeof(subproblem_t));
	sp->column = problem->column + 1;
	sp->solution = (int*)malloc(problem->N * sizeof(int));
	memcpy(sp->solution, problem->solution, problem->N * sizeof(int));

	Pheet::template spawn<ParallelRecursiveNQueensTask<Pheet> >(sp);
      }
    }
  }
  /* Free resources. These are not shared with the children. */
  free(problem->solution);
  free(problem);
}

}

#endif /* PARALLELRECURSIVENQUEENSTASK_H_ */
