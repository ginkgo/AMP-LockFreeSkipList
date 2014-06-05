/*
 * ExponentialBackoff.h
 *
 *  Created on: 18.04.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef EXPONENTIALBACKOFF_H_
#define EXPONENTIALBACKOFF_H_

#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <thread>
#include <chrono>

/*
 *
 */
namespace pheet {

template <class Pheet, unsigned int MIN_BACKOFF = 100, unsigned int MAX_BACKOFF = 100000>
class ExponentialBackoffImpl {
public:
	ExponentialBackoffImpl();
	~ExponentialBackoffImpl();

	void backoff();
	void reset() {
		limit = MIN_BACKOFF;
	}
private:
	unsigned int limit;
};

template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
ExponentialBackoffImpl<Pheet, MIN_BACKOFF, MAX_BACKOFF>::ExponentialBackoffImpl() {
	limit = MIN_BACKOFF;
}

template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
ExponentialBackoffImpl<Pheet, MIN_BACKOFF, MAX_BACKOFF>::~ExponentialBackoffImpl() {
}

template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
void ExponentialBackoffImpl<Pheet, MIN_BACKOFF, MAX_BACKOFF>::backoff() {
	Pheet p;
	unsigned int sleep = p.rand_int(limit); //rnd_gen(rng);

	std::this_thread::sleep_for(std::chrono::nanoseconds(sleep));
	
	limit = std::min(limit + sleep, MAX_BACKOFF);
}

template <class Pheet>
using ExponentialBackoff = ExponentialBackoffImpl<Pheet, 100, 100000>;

}

#endif /* EXPONENTIALBACKOFF_H_ */
