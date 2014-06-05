/*
 * NQueensTests.h
 *
 *  Created on: 2011-09-22
 *      Author: Anders Gidenstam, Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef NQUEENSTESTS_H_
#define NQUEENSTESTS_H_

#include "../Test.h"
#include "../init.h"
#ifdef NQUEENS_TEST
#include "NQueensTest.h"
#endif

namespace pheet {

class NQueensTests : Test {
public:
	NQueensTests();
	virtual ~NQueensTests();

	void run_test();

private:
	template<class Solver>
	void run_solver();
};


template <class Solver>
void NQueensTests::run_solver() {
#ifdef NQUEENS_TEST
  for(size_t n = 0; n < sizeof(nqueens_test_n)/sizeof(nqueens_test_n[0]); n++) {
    for(size_t c = 0; c < sizeof(nqueens_test_cpus)/sizeof(nqueens_test_cpus[0]); c++) {
      if(nqueens_test_cpus[c] <= Solver::max_cpus) {
	NQueensTest<Solver> nt(nqueens_test_cpus[c], nqueens_test_n[n]);
	nt.run_test();
      }
    }
  }
#endif
}

}

#endif /* NQUEENSTESTS_H_ */
