/*
 * TASLock.h
 *
 *  Created on: 10.08.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TTASLOCK_H_
#define TTASLOCK_H_

#include <iostream>
#include <chrono>
#include <atomic>
#include "../common/BasicLockGuard.h"

namespace pheet {

template <class Pheet>
class TTASLock {
public:
	typedef TTASLock<Pheet> Self;
	typedef BasicLockGuard<Pheet, Self> LockGuard;

	TTASLock();
	~TTASLock();

	void lock();
	bool try_lock();
	bool try_lock(long int time_ms);

	void unlock();

	static void print_name() {
		std::cout << "TTASLock";
	}
private:
	// Volatile needed to ensure the compiler doesn't optimize the while(locked) loop
	std::atomic<int> locked;
};

template <class Pheet>
TTASLock<Pheet>::TTASLock()
:locked(0) {

}

template <class Pheet>
TTASLock<Pheet>::~TTASLock() {
}

template <class Pheet>
void TTASLock<Pheet>::lock() {
	int expect;
	do {
		while(locked.load(std::memory_order_relaxed));
		expect = 0;
	}
	while(!locked.compare_exchange_weak(expect, 1, std::memory_order_acquire));
}

template <class Pheet>
bool TTASLock<Pheet>::try_lock() {
	int expect = 0;
	return (!locked.load(std::memory_order_relaxed)) && locked.compare_exchange_strong(expect, 1, std::memory_order_acquire);
}

template <class Pheet>
bool TTASLock<Pheet>::try_lock(long int time_ms) {
	auto start_time = std::chrono::high_resolution_clock::now();
	int expect = 0;
	while(!locked.compare_exchange_strong(expect, 1, std::memory_order_acquire)) {
		do {
			auto stop_time = std::chrono::high_resolution_clock::now();
			long int cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
			if(cur_time >= time_ms) {
				return false;
			}
		}while(locked.load(std::memory_order_relaxed));
		expect = 0;
	};
	return true;
}

template <class Pheet>
void TTASLock<Pheet>::unlock() {
	locked.store(0, std::memory_order_release);
}

}

#endif /* TASLOCK_H_ */
