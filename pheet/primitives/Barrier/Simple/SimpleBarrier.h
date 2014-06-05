/*
 * SimpleBarrier.h
 *
 *  Created on: 18.04.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SIMPLEBARRIER_H_
#define SIMPLEBARRIER_H_

#include "../../../misc/types.h"
#include <atomic>

/*
 *
 */
namespace pheet {

template <class Pheet>
class SimpleBarrier {
public:
	SimpleBarrier();
	~SimpleBarrier();

	void wait(size_t i, procs_t p);
	void signal(size_t i);
	void barrier(size_t i, procs_t p);
	void reset();

private:

	typedef typename Pheet::Backoff Backoff;

	std::atomic<procs_t> barriers[4];
};

template <class Pheet>
SimpleBarrier<Pheet>::SimpleBarrier() {
	reset();
}

template <class Pheet>
SimpleBarrier<Pheet>::~SimpleBarrier() {

}

template <class Pheet>
void SimpleBarrier<Pheet>::signal(size_t i) {
	// Of course requires that no more than 2 phases of the barrier overlap
	// If this is required, a more complex barrier implementation is necessary
	barriers[(i+2) & 3].store(0, std::memory_order_relaxed);

	// All writes happening before should become visible
	barriers[i&3].fetch_add(1, std::memory_order_release);
}

template <class Pheet>
void SimpleBarrier<Pheet>::wait(size_t i, procs_t p) {
	Backoff bo;
	// All writes to the barrier need to have happened before we continue
	while(barriers[i&3].load(std::memory_order_acquire) != p) {
		bo.backoff();
	}
}

template <class Pheet>
void SimpleBarrier<Pheet>::barrier(size_t i, procs_t p) {
	signal(i);
	wait(i, p);
}

template <class Pheet>
void SimpleBarrier<Pheet>::reset() {
	barriers[0].store(0, std::memory_order_relaxed);
	barriers[1].store(0, std::memory_order_relaxed);
	barriers[2].store(0, std::memory_order_relaxed);
	barriers[3].store(0, std::memory_order_relaxed);
}

}

#endif /* SIMPLEBARRIER_H_ */
