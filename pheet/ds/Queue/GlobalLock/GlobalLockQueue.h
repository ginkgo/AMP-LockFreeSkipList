/*
 * GlobalLockQueue.h
 *
 *  Created on: May 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef GLOBALLOCKQUEUE_H_
#define GLOBALLOCKQUEUE_H_

#include <queue>
#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, typename TT>
class GlobalLockQueue {
public:
	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;

	GlobalLockQueue()
	: length(0){}
	~GlobalLockQueue() {}

	void push(TT const& item) {
		LockGuard g(m);

		data.push(item);
		length.store(length.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
	}

	bool try_pop(TT& ret) {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return false;
		}

		ret = data.front();

		data.pop();
		length.store(length.load(std::memory_order_relaxed) - 1, std::memory_order_relaxed);
		return true;
	}

	inline size_t size() const {
		return length.load(std::memory_order_relaxed);
	}

	static void print_name() {
		std::cout << "GlobalLockQueue<";
		Mutex::print_name();
		std::cout << ">";
	}

private:
	std::queue<TT> data;
	std::atomic<size_t> length;
	Mutex m;
};

} /* namespace pheet */
#endif /* GLOBALLOCKQUEUE_H_ */
