/*
 * GlobalLockStack.h
 *
 *  Created on: May 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef GLOBALLOCKSTACK_H_
#define GLOBALLOCKSTACK_H_

#include <vector>
#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, typename TT>
class GlobalLockStack {
public:
	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;

	GlobalLockStack()
	: length(0){}
	~GlobalLockStack() {}

	void push(TT const& item) {
		LockGuard g(m);

		data.push_back(item);
		length.store(length.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
	}

	bool try_pop(TT& ret) {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return false;
		}

		ret = data.back();

		data.pop_back();
		length.store(length.load(std::memory_order_relaxed) - 1, std::memory_order_relaxed);
		return true;
	}

	inline size_t get_length() const {
		return length;
	}

	inline size_t size() const {
		return get_length();
	}

	static void print_name() {
		std::cout << "GlobalLockStack<";
		Mutex::print_name();
		std::cout << ">";
	}

private:
	std::vector<TT> data;
	std::atomic<size_t> length;
	Mutex m;
};

} /* namespace pheet */
#endif /* GLOBALLOCKSTACK_H_ */
