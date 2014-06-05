/*
 * MixedModeQuicksortTask.h
 *
 *  Created on: 30.07.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MIXEDMODEQUICKSORTTASK_H_
#define MIXEDMODEQUICKSORTTASK_H_

#include <pheet/pheet.h>
#include "../MixedMode/MixedModeQuicksortTask.h"
#include "../Dag/DagQuicksort.h"

#include <algorithm>

namespace pheet {

template <class Pheet, size_t BLOCK_SIZE = 4096>
class MixedModeQuicksortTask : public Pheet::Task {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::Barrier Barrier;

	MixedModeQuicksortTask(unsigned int* data, size_t length);
	virtual ~MixedModeQuicksortTask();

	virtual void operator()();

private:
	void partition();
	void neutralize(ptrdiff_t &leftPos, ptrdiff_t &leftEnd, ptrdiff_t &rightPos, ptrdiff_t &rightEnd);
	bool is_partitioned();
	void assert_is_partitioned();

	unsigned int* data;
	size_t length;

	unsigned int pivot;
	size_t pivotPosition;

	// Aligned starting positions in the array
	ptrdiff_t leftStart;
	ptrdiff_t rightStart;

	// the leftmost unclaimed blocks after the initial team_size blocks
	ptrdiff_t leftBlock;
	// the rightmost unclaimed blocks after the initial team_size blocks
	ptrdiff_t rightBlock;

	// The number of blocks still to process + the initial blocks (which are not subtracted)
	ptrdiff_t remainingBlocks;
	procs_t threads_finished_left;
	procs_t threads_finished_right;

	ptrdiff_t remainingLeft;
	ptrdiff_t remainingRight;

	Barrier barrier;
};

template <class Pheet, size_t BLOCK_SIZE>
MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::MixedModeQuicksortTask(unsigned int* data, size_t length)
: data(data), length(length) {
	pivotPosition = -1;

	ptrdiff_t pivotIndex = length - 1; //rand() % length;
	pivot = data[pivotIndex];

	// assuming BLOCK_SIZE is a power of two
	ptrdiff_t missalign = (ptrdiff_t)(((size_t)data) & (BLOCK_SIZE - 1));
	leftStart = -missalign;
	ptrdiff_t tmp = length - 2;
	ptrdiff_t rightMissalign = (tmp + missalign) & (BLOCK_SIZE - 1);
	if(rightMissalign == 0)
		rightMissalign = BLOCK_SIZE;
	rightStart = tmp - rightMissalign + BLOCK_SIZE - 1;
	if(rightStart < tmp)
		rightStart += BLOCK_SIZE;
/*	if(rightStart >= (tmp + BLOCK_SIZE))
		cout << "wrong rightstart" << endl;*/
/*	if(((rightStart + 1 - leftStart) % BLOCK_SIZE) != 0)
		cout << "bad alignment" << ((rightStart + 1 - leftStart) % BLOCK_SIZE) << endl;
		*/
	// the first block is always given to the coordinating thread
	leftBlock = 1;
	rightBlock = 1;
	remainingBlocks = ((rightStart + 1 - leftStart) / BLOCK_SIZE) - (2);
	threads_finished_left = 0;
	threads_finished_right = 0;

	remainingLeft = length;
	remainingRight = length;
}

template <class Pheet, size_t BLOCK_SIZE>
MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::~MixedModeQuicksortTask() {

}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::operator()() {
	procs_t team_size = Pheet::Environment::get_place()->get_team_size();
	if(team_size == 1) {
		// For np == 1 switch to dag quicksort
		Pheet::template call<DagQuicksort<Pheet>>(data, length);
		return;
	}

	partition();

	barrier.barrier(0, team_size);
	MEMORY_FENCE();
	assert_is_partitioned();

	procs_t team_max = std::min(team_size, ((length / BLOCK_SIZE) / 8) + 2);
	procs_t team1 = (pivotPosition * team_max) / length;
	if(team1 == 0) {
		team1 = 1;
	}
	pheet_assert(team1 <= team_max);
	procs_t team2 = team_max - team1;
	Pheet::Environment::template spawn_nt<MixedModeQuicksortTask<Pheet, BLOCK_SIZE> >(team1, data, pivotPosition);
	Pheet::Environment::template spawn_nt<MixedModeQuicksortTask<Pheet, BLOCK_SIZE>>(team2, data + pivotPosition + 1, length - pivotPosition - 1);
}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::partition() {
	procs_t localId = Pheet::Environment::get_place()->get_local_id();
	procs_t team_size = Pheet::Environment::get_place()->get_team_size();

	ptrdiff_t localLeftBlock = localId;
	ptrdiff_t localRightBlock = localId;

	ptrdiff_t leftPos;
	ptrdiff_t rightPos;
	ptrdiff_t leftEnd;
	ptrdiff_t rightEnd;
	if(Pheet::Environment::get_place()->is_coordinator()) {
		// Special case for corner blocks, as they might not have the standard block size
		leftPos = 0;
		rightPos = length - 2;
		leftEnd = leftStart + BLOCK_SIZE;
		rightEnd = rightStart - BLOCK_SIZE;
	}
	else {
		// No block for those
		leftPos = 0;
		rightPos = length;
		leftEnd = 0;
		rightEnd = length;
	/*	leftPos = leftStart + localLeftBlock * BLOCK_SIZE;
		rightPos = rightStart - localRightBlock * BLOCK_SIZE;
		leftEnd = leftPos + BLOCK_SIZE;
		rightEnd = rightPos - BLOCK_SIZE;*/
	}

	while(true) {
		neutralize(leftPos, leftEnd, rightPos, rightEnd);

		if(leftPos == leftEnd) {
			int tmp = INT_FETCH_AND_SUB(&remainingBlocks, 1);
			if(tmp > 0) {
				localLeftBlock = INT_FETCH_AND_ADD(&leftBlock, 1);
				leftPos = leftStart + localLeftBlock * BLOCK_SIZE;
				leftEnd = leftPos + BLOCK_SIZE;
			}
			else {
		//		INT_ATOMIC_ADD(&threads_finished_left, 1);
				break;
			}
		}
		if(rightPos == rightEnd) {
			int tmp = INT_FETCH_AND_SUB(&remainingBlocks, 1);
			if(tmp > 0) {
				localRightBlock = INT_FETCH_AND_ADD(&rightBlock, 1);
				rightPos = rightStart - localRightBlock * BLOCK_SIZE;
				rightEnd = rightPos - BLOCK_SIZE;
			}
			else {
		//		INT_ATOMIC_ADD(&threads_finished_right, 1);
				break;
			}
		}
	}

	// Now we need the complete team
	Pheet::Environment::get_place()->sync_team();

	while(true) {
		if(leftPos == leftEnd) {
			INT_ATOMIC_ADD(&threads_finished_left, 1);

			Backoff bo;
			while(true) {
				ptrdiff_t tmp = remainingLeft;
				if(tmp != (ptrdiff_t)length && PTRDIFFT_CAS(&(remainingLeft), tmp, length)) {
					leftPos = tmp;
					leftEnd = leftPos - ((leftPos - leftStart) % BLOCK_SIZE) + BLOCK_SIZE;
					break;
				}
				else if((team_size - threads_finished_left) <= (unsigned)(localId)) {
					if(localId == 0) {
						break;
					}
					else if(rightPos == rightEnd) {
						INT_ATOMIC_ADD(&threads_finished_right, 1);
						break;
					}
					else if(remainingRight == (ptrdiff_t)length &&
							(PTRDIFFT_CAS(&(remainingRight), (ptrdiff_t)length, rightPos))) {
						break;
					}
					else {
						bo.backoff();
					}
				}
			}
			if(leftPos == leftEnd) {
				break;
			}
		}
		if(rightPos == rightEnd) {
			INT_ATOMIC_ADD(&threads_finished_right, 1);

			Backoff bo;
			while(true) {
				ptrdiff_t tmp = remainingRight;
				if(tmp != (ptrdiff_t)length && PTRDIFFT_CAS(&(remainingRight), tmp, length)) {
					rightPos = tmp;
					rightEnd = (rightPos - ((rightPos - leftStart) % BLOCK_SIZE)) - 1;
					break;
				}
				else if((team_size - threads_finished_right) <= (unsigned)(localId)) {
					if(localId == 0) {
						break;
					}
					else if(leftPos == leftEnd) {
						INT_ATOMIC_ADD(&threads_finished_left, 1);
						break;
					}
					else if(remainingLeft == (ptrdiff_t)length &&
							(PTRDIFFT_CAS(&(remainingLeft), (ptrdiff_t)length, leftPos))) {
						break;
					}
					else {
						bo.backoff();
					}
				}
			}
			if(rightPos == rightEnd) {
				break;
			}
		}

		neutralize(leftPos, leftEnd, rightPos, rightEnd);
	}

	if(localId == 0) {
		ptrdiff_t pp;
		if(leftPos == leftEnd) {
			procs_t rf = team_size;
			pp = leftStart + leftBlock * BLOCK_SIZE;
			pheet_assert(pp >= 0);

			while(true) {
				if(pp >= rightPos || rightPos == rightEnd) {
					--rf;

					Backoff bo;
					bool another_pos = false;
					while(rf > threads_finished_right) {
						ptrdiff_t tmp = remainingRight;
						if(tmp != (ptrdiff_t)length && PTRDIFFT_CAS(&(remainingRight), tmp, length)) {
							rightPos = tmp;
							rightEnd = (rightPos - ((rightPos - leftStart) % BLOCK_SIZE)) - 1;
							another_pos = true;
							break;
						}
						bo.backoff();
					}
					if(!another_pos) {
						break;
					}
				}

				// neutralize
				while(true) {
					while(rightPos > rightEnd) {
						if(data[rightPos] < pivot)
							break;
						rightPos--;
					}
					if(rightPos > rightEnd) {
						while(pp < rightPos) {
							if(data[pp] > pivot)
								break;
							++pp;
						}
					}
					if(pp >= rightPos || rightPos == rightEnd) {
						break;
					}
					else {
						std::swap(data[pp], data[rightPos]);
					}
				}
			}

			if(data[pp] < pivot) {
				++pp;
			}
		}
		else { /* rightPos == rightEnd */
			pheet_assert(rightPos == rightEnd);

			procs_t lf = team_size;
			pp = rightStart - rightBlock * BLOCK_SIZE;
			pheet_assert(pp <= (ptrdiff_t)length - 2);

			while(true) {
				if(leftPos >= pp || leftPos == leftEnd) {
					--lf;

					Backoff bo;
					bool another_pos = false;
					while(lf > threads_finished_left) {
						ptrdiff_t tmp = remainingLeft;
						if(tmp != (ptrdiff_t)length && PTRDIFFT_CAS(&(remainingLeft), tmp, length)) {
							leftPos = tmp;
							leftEnd = leftPos - ((leftPos - leftStart) % BLOCK_SIZE) + BLOCK_SIZE;
							another_pos = true;
							break;
						}
						bo.backoff();
					}
					if(!another_pos) {
						break;
					}
				}

				// neutralize
				while(true) {
					while(leftPos < leftEnd) {
						if(data[leftPos] > pivot)
							break;
						leftPos++;
					}
					if(leftPos < leftEnd) {
						while(leftPos < pp) {
							if(data[pp] < pivot)
								break;
							--pp;
						}
					}
					if(leftPos >= pp || leftPos == leftEnd) {
						break;
					}
					else {
						std::swap(data[leftPos], data[pp]);
					}
				}
			}

			if(data[pp] < pivot) {
				++pp;
			}
		}

		pheet_assert(pp >= 0 && pp < (ptrdiff_t)length);
		pheet_assert(data[pp] >= pivot);
		pheet_assert(pp == 0 || data[pp-1] <= pivot);

		if(pp < (((ptrdiff_t)length) - 1)) {
			std::swap(data[length - 1], data[pp]);
		}
		MEMORY_FENCE();
		pivotPosition = pp;
	//	cout << "pivot pos: " << pivotPosition << endl;
	//	cout << "Checking results" << endl;

	}
}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::neutralize(ptrdiff_t &leftPos, ptrdiff_t &leftEnd, ptrdiff_t &rightPos, ptrdiff_t &rightEnd) {
	while(true) {
		while(leftPos < leftEnd) {
			if(data[leftPos] > pivot)
				break;
			leftPos++;
		}
		while(rightPos > rightEnd) {
			if(data[rightPos] < pivot)
				break;
			rightPos--;
		}
		if(leftPos == leftEnd || rightPos == rightEnd)
			break;
		pheet_assert(leftPos < rightPos && leftPos >= 0 && rightPos < (((ptrdiff_t)length) - 1));
		std::swap(data[leftPos], data[rightPos]);
	}
}

template <class Pheet, size_t BLOCK_SIZE>
bool MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::is_partitioned() {
	for(size_t i = 0; i < length; i++) {
		if(i < pivotPosition && data[i] > pivot) {
			return false;
		//	cout << "data too large! " << i << endl;
		}
		else if(i > pivotPosition && data[i] < pivot) {
			return false;
		//	cout << "data too small! " << i << endl;
		}
		else if(i == pivotPosition && data[i] != pivot) {
			return false;
		/*	cout << "wrong pivot at " << data << " "<< (data + i) << ": " << i << " (" << (leftStart + leftBlock * BLOCK_SIZE) << ") " << pivot << " [" << data[i - 1] << "," << data[i] << "," << data[i+1] << "] " << data[length - 1] << " [" << leftPos << "," << rightPos << "] " << length << endl;
			cout << "tmp " << tmp << " tmp2 " << tmp2 << endl;
			cout << "right start " << rightStart << " length " << length << endl;
			for(int j = 0; j < length; j++) {
				if(data[j] == pivot) {
					cout << "pivot at " << j << endl;
				}
			}
			*/
		}
	}
	return true;
}

template <class Pheet, size_t BLOCK_SIZE>
void MixedModeQuicksortTask<Pheet, BLOCK_SIZE>::assert_is_partitioned() {
	for(size_t i = 0; i < length; i++) {
		pheet_assert(i >= pivotPosition || data[i] <= pivot);
		pheet_assert(i <= pivotPosition || data[i] >= pivot);
		pheet_assert(i != pivotPosition || data[i] == pivot);
	}
}

}

#endif /* MIXEDMODEQUICKSORTTASK_H_ */
