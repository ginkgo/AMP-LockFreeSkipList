/*
 * SimpleLUPiv.h
 *
 *  Created on: Dec 20, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SIMPLELUPIV_H_
#define SIMPLELUPIV_H_

#include "../helpers/LUPivPivotTask.h"
#include "SimpleLUPivStandardPathTask.h"
#include "SimpleLUPivCriticalPathTask.h"
#include "SimpleLUPivPerformanceCounters.h"

#include <algorithm>

extern "C" {
void dgetrf_(int *m, int *n, double *a, int *lda, int *piv, int *info);
void dgetf2_(int *m, int *n, double *a, int *lda, int *piv, int *info);
}

namespace pheet {

template <class Pheet, int BLOCK_SIZE = 128>
class SimpleLUPivImpl : public Pheet::Task {
public:
	typedef SimpleLUPivPerformanceCounters<Pheet> PerformanceCounters;

	SimpleLUPivImpl(double* a, int* pivot, int m, int lda, int n);
	SimpleLUPivImpl(double* a, int* pivot, int size, PerformanceCounters& pc);
	~SimpleLUPivImpl();

	virtual void operator()();

	static char const name[];
private:
	// The matrix (column-major)
	double* a;
	// vector containing the pivot indices for the rows (length: m)
	int* pivot;
	// Number of rows in a
	int m;
	// Leading dimension (lda >= max(1, m))
	int lda;
	// Number of columns in a
	int n;
};

template <class Pheet, int BLOCK_SIZE>
char const SimpleLUPivImpl<Pheet, BLOCK_SIZE>::name[] = "SimpleLUPiv";

template <class Pheet, int BLOCK_SIZE>
SimpleLUPivImpl<Pheet, BLOCK_SIZE>::SimpleLUPivImpl(double* a, int* pivot, int m, int lda, int n)
: a(a), pivot(pivot), m(m), lda(lda), n(n) {
	pheet_assert(m > 0);
	pheet_assert(n > 0);
	pheet_assert(lda >= m);
	// TODO: debug cases n != m  - until then:
	pheet_assert(m == n);
}

template <class Pheet, int BLOCK_SIZE>
SimpleLUPivImpl<Pheet, BLOCK_SIZE>::SimpleLUPivImpl(double* a, int* pivot, int size, PerformanceCounters& pc)
: a(a), pivot(pivot), m(size), lda(size), n(size) {
	pheet_assert(m > 0);
	pheet_assert(n > 0);
	pheet_assert(lda >= m);
}

template <class Pheet, int BLOCK_SIZE>
SimpleLUPivImpl<Pheet, BLOCK_SIZE>::~SimpleLUPivImpl() {

}

template <class Pheet, int BLOCK_SIZE>
void SimpleLUPivImpl<Pheet, BLOCK_SIZE>::operator()() {
	int num_blocks = std::min(n, m) / BLOCK_SIZE;

	// Run sequential algorithm on first column
	int out = 0;
	int tmp = std::min(BLOCK_SIZE, n);
	dgetf2_(&m, &tmp, a, &lda, pivot, &out);

	if(num_blocks > 1) {
		double* cur_a = a;
		int* cur_piv = pivot;
		int cur_m = m;
		/*
		 * Algorithm by blocks. For each iteration perform this work
		 * Could be improved to be even more finegrained with a dag, where some dgemm tasks can still be performed
		 * while we already execute the next iteration
		 */
		for(int i = 1; i < num_blocks; ++i) {
			// Finish one iteration before you can start the next one
			{typename Pheet::Finish f;
				// Critical path
				Pheet::template
					spawn<SimpleLUPivCriticalPathTask<Pheet, BLOCK_SIZE> >(cur_a + i*BLOCK_SIZE*lda, cur_a + (i-1)*BLOCK_SIZE*lda, cur_piv, cur_m, lda, (i == num_blocks - 1)?(n - BLOCK_SIZE*i):(BLOCK_SIZE));

				// Workflow for other unfinished columns
				for(int j = i + 1; j < num_blocks; ++j) {
					Pheet::template
						spawn<SimpleLUPivStandardPathTask<Pheet, BLOCK_SIZE> >(cur_a + j*BLOCK_SIZE*lda, cur_a + (i-1)*BLOCK_SIZE*lda, cur_piv, cur_m, lda, (j == num_blocks - 1)?(n - BLOCK_SIZE*j):(BLOCK_SIZE));
				}

				// Pivoting for all other columns
				for(int j = 0; j < (i-1); ++j) {
					Pheet::template
						spawn<LUPivPivotTask<Pheet> >(cur_a + j*BLOCK_SIZE*lda, cur_piv, std::min(cur_m, BLOCK_SIZE), lda, BLOCK_SIZE);
				}

				cur_a += BLOCK_SIZE;
				cur_piv += BLOCK_SIZE;
				cur_m -= BLOCK_SIZE;
			}
		}
		{typename Pheet::Finish f;
			// Pivoting for all other columns
			for(int j = 0; j < (num_blocks-1); ++j) {
				Pheet::template
					spawn<LUPivPivotTask<Pheet> >(cur_a + j*BLOCK_SIZE*lda, cur_piv, std::min(cur_m, BLOCK_SIZE), lda, BLOCK_SIZE);
			}
		}

		// Update pivots as the offsets are calculated from the beginning of the block
		for(int i = BLOCK_SIZE; i < m; i += BLOCK_SIZE) {
			for(int j = i; j < i+BLOCK_SIZE; j++) {
				pheet_assert(pivot[j] != 0);
				pheet_assert(pivot[j] <= m-i);
				pivot[j] = pivot[j] + i;
				pheet_assert(pivot[j] >= j+1);
			}
		}
	}
}

template <class Pheet>
using SimpleLUPiv = SimpleLUPivImpl<Pheet, 128>;

}

#endif /* SIMPLELUPIV_H_ */
