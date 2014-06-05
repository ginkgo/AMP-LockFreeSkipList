/*
 * LocalityStrategyLUPiv.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPLOCALITYSTRATEGYLUPIV_H_
#define PPOPPLOCALITYSTRATEGYLUPIV_H_

#include "../LocalityStrategy/LocalityStrategyLUPivPivotTask.h"
#include "PPoPPLocalityStrategyLUPivPerformanceCounters.h"
#include "PPoPPLocalityStrategyLUPivStandardPathTask.h"
#include "PPoPPLocalityStrategyLUPivCriticalPathTask.h"
#include "PPoPPLUPivLocalityStrategy.h"

#include <algorithm>

extern "C" {
void dgetrf_(int *m, int *n, double *a, int *lda, int *piv, int *info);
void dgetf2_(int *m, int *n, double *a, int *lda, int *piv, int *info);
}

namespace pheet {

template <class Pheet, int BLOCK_SIZE, bool s2c>
class PPoPPLocalityStrategyLUPivImpl : public Pheet::Task {
public:
	typedef PPoPPLocalityStrategyLUPivPerformanceCounters<Pheet> PerformanceCounters;

	PPoPPLocalityStrategyLUPivImpl(double* a, int* pivot, int m, int lda, int n);
	PPoPPLocalityStrategyLUPivImpl(double* a, int* pivot, int size, PerformanceCounters& pc);
	~PPoPPLocalityStrategyLUPivImpl();

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

	PerformanceCounters pc;
};

template <class Pheet, int BLOCK_SIZE, bool s2c>
char const PPoPPLocalityStrategyLUPivImpl<Pheet, BLOCK_SIZE, s2c>::name[] = "PPoPPLocalityStrategyLUPiv";

template <class Pheet, int BLOCK_SIZE, bool s2c>
PPoPPLocalityStrategyLUPivImpl<Pheet, BLOCK_SIZE, s2c>::PPoPPLocalityStrategyLUPivImpl(double* a, int* pivot, int m, int lda, int n)
: a(a), pivot(pivot), m(m), lda(lda), n(n) {
	pheet_assert(m > 0);
	pheet_assert(n > 0);
	pheet_assert(lda >= m);
	// TODO: debug cases n != m  - until then:
	pheet_assert(m == n);
}

template <class Pheet, int BLOCK_SIZE, bool s2c>
PPoPPLocalityStrategyLUPivImpl<Pheet, BLOCK_SIZE, s2c>::PPoPPLocalityStrategyLUPivImpl(double* a, int* pivot, int size, PerformanceCounters& pc)
: a(a), pivot(pivot), m(size), lda(size), n(size), pc(pc) {
	pheet_assert(m > 0);
	pheet_assert(n > 0);
	pheet_assert(lda >= m);
}

template <class Pheet, int BLOCK_SIZE, bool s2c>
PPoPPLocalityStrategyLUPivImpl<Pheet, BLOCK_SIZE, s2c>::~PPoPPLocalityStrategyLUPivImpl() {

}

template <class Pheet, int BLOCK_SIZE, bool s2c>
void PPoPPLocalityStrategyLUPivImpl<Pheet, BLOCK_SIZE, s2c>::operator()() {
	int num_blocks = std::min(n, m) / BLOCK_SIZE;

	// Run sequential algorithm on first column
	int out = 0;
	int tmp = std::min(BLOCK_SIZE, n);
	dgetf2_(&m, &tmp, a, &lda, pivot, &out);

	if(num_blocks > 1) {
		double* cur_a = a;
		int* cur_piv = pivot;
		int cur_m = m;

		typename Pheet::Place** column_owners = new typename Pheet::Place*[num_blocks];
		typename Pheet::Place** block_owners = new typename Pheet::Place*[num_blocks*num_blocks];
		typename Pheet::Place* local_place = Pheet::get_place();
		for(int i = 0; i < num_blocks; ++i) {
			column_owners[i] = local_place;
			for(int j = 0; j < num_blocks; ++j) {
				block_owners[j + i*num_blocks] = Pheet::get_place();
			}
		}
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
					spawn_s<PPoPPLocalityStrategyLUPivCriticalPathTask<Pheet, BLOCK_SIZE, s2c> >(
							PPoPPLUPivLocalityStrategy<Pheet, s2c>(column_owners[i], num_blocks - i, true),
							column_owners + i, block_owners + (i-1) + (i*num_blocks),
							cur_a + i*BLOCK_SIZE*lda, cur_a + (i-1)*BLOCK_SIZE*lda, cur_piv, cur_m, lda, (i == num_blocks - 1)?(n - BLOCK_SIZE*i):(BLOCK_SIZE), pc);

				// Workflow for other unfinished columns
				for(int j = i + 1; j < num_blocks; ++j) {
					Pheet::template
						spawn_s<PPoPPLocalityStrategyLUPivStandardPathTask<Pheet, BLOCK_SIZE, s2c> >(
								PPoPPLUPivLocalityStrategy<Pheet, s2c>(column_owners[j], num_blocks - i, false),
								column_owners + j, block_owners + (i-1) + (j*num_blocks),
								cur_a + j*BLOCK_SIZE*lda, cur_a + (i-1)*BLOCK_SIZE*lda, cur_piv, cur_m, lda, (j == num_blocks - 1)?(n - BLOCK_SIZE*j):(BLOCK_SIZE), pc);
				}

				// Pivoting for all other columns
				for(int j = 0; j < (i-1); ++j) {
					Pheet::template
						spawn_s<LocalityStrategyLUPivPivotTask<Pheet> >(
								PPoPPLUPivLocalityStrategy<Pheet, s2c>(column_owners[j], 1, false),
								column_owners + j,
								cur_a + j*BLOCK_SIZE*lda, cur_piv, std::min(cur_m, BLOCK_SIZE), lda, BLOCK_SIZE);
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
					spawn_s<LocalityStrategyLUPivPivotTask<Pheet> >(
							PPoPPLUPivLocalityStrategy<Pheet, s2c>(column_owners[j], 1, false),
							column_owners + j,
							cur_a + j*BLOCK_SIZE*lda, cur_piv, std::min(cur_m, BLOCK_SIZE), lda, BLOCK_SIZE);
			}
		}

		delete[] column_owners;
		delete[] block_owners;

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
using PPoPPLocalityStrategyLUPiv = PPoPPLocalityStrategyLUPivImpl<Pheet, 128, true>;
template <class Pheet>
using PPoPPLocalityStrategyLUPivNoS2C = PPoPPLocalityStrategyLUPivImpl<Pheet, 128, false>;

}

#endif /* PPOPPLOCALITYSTRATEGYLUPIV_H_ */
