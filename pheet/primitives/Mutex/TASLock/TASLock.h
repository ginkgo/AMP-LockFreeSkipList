/*
 * TASLock.h
 *
 *  Created on: 22.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TASLOCK_H_
#define TASLOCK_H_

#include <iostream>
#include <chrono>
#include <atomic>
#include "../common/BasicLockGuard.h"

namespace pheet {

template <class Pheet>
class TASLock {
public:
	typedef TASLock<Pheet> Self;
	typedef BasicLockGuard<Pheet, Self> LockGuard;

	TASLock();
	~TASLock();

	void lock();
	bool try_lock();
	bool try_lock(long int time_ms);

	void unlock();

	static void print_name() {
		std::cout << "TASLock";
	}
private:
	std::atomic<int> locked;
};

template <class Pheet>
TASLock<Pheet>::TASLock()
:locked(0) {

}

template <class Pheet>
TASLock<Pheet>::~TASLock() {
}

template <class Pheet>
void TASLock<Pheet>::lock() {
	int expect = 0;
	while(!locked.compare_exchange_weak(expect, 1, std::memory_order_acquire)) {
		expect = 0;
	};
}

template <class Pheet>
bool TASLock<Pheet>::try_lock() {
	int expect = 0;
	return locked.compare_exchange_strong(expect, 1, std::memory_order_acquire);
}

template <class Pheet>
bool TASLock<Pheet>::try_lock(long int time_ms) {
	auto start_time = std::chrono::high_resolution_clock::now();
	int expect = 0;
	while(!locked.compare_exchange_strong(expect, 1, std::memory_order_acquire)) {
		auto stop_time = std::chrono::high_resolution_clock::now();
		long int cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
		if(cur_time >= time_ms) {
			return false;
		}
		expect = 0;
	};
	return true;
}

template <class Pheet>
void TASLock<Pheet>::unlock() {
	locked.store(0, std::memory_order_release);
}


}

#endif /* TASLOCK_H_ */
