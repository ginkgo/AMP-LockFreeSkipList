/*
 * NQueensTest.h
 *
 *  Created on: 2011-09-22
 *      Author: Anders Gidenstam, Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef NQUEENSTEST_H_
#define NQUEENSTEST_H_

#include <cstdlib>
#include <iostream>
#include "../../pheet/misc/types.h"
#include "../Test.h"
#include <exception>

using namespace std;

namespace pheet {

template <class Solver>
class NQueensTest : Test {
public:
    NQueensTest(procs_t cpus, size_t size);
    virtual ~NQueensTest();

    void run_test();

private:
    bool validate(int* solution);

    procs_t cpus;
    size_t size;
};

template <class Solver>
NQueensTest<Solver>::NQueensTest(procs_t cpus, size_t size)
: cpus(cpus), size(size) {

}

template <class Solver>
NQueensTest<Solver>::~NQueensTest() {

}

template <class Solver>
void NQueensTest<Solver>::run_test() {
  Solver s(cpus, size);
  int*   solution;

  Time start, end;
  check_time(start);
  solution = s.solve();
  check_time(end);

  double seconds = calculate_seconds(start, end);
  std::cout << "test\tsolver\tscheduler\tsize\t\tcpus\ttotal_time\tvalid\t";
  s.print_headers();
  std::cout << std::endl;
  cout << "N-queens\t" << Solver::name << "\t" << Solver::scheduler_name << "\t" << "\t" << size << "\t" << cpus << "\t" << seconds << "\t" << validate(solution) << "\t";
  s.print_results();
  cout << endl;

  free(solution);
}

template <class Solver>
bool NQueensTest<Solver>::validate(int* solution) {
  if (solution == 0) {
    cerr << "No solution found for " << size << "x" << size << " instance." << endl;
    return (size < 3); // Presuming all instances for N > 3 has a solution.
  }

  for (size_t c = 0; c < size; c++) {
    for (int n = 1; n < (int)size - (int)c; n++)
      if ((solution[c + n] == solution[c] - n) ||
	  (solution[c + n] == solution[c]) ||
	  (solution[c + n] == solution[c] + n)) {
	return 0;
      }
  }
  return 1;
}

}

#endif /* NQUEENSTEST_H_ */
