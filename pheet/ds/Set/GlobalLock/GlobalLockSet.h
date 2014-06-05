/*
 * GlobalLockSet.h
 *
 *  Created on: May 30, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef GLOBALLOCKSET_H_
#define GLOBALLOCKSET_H_

#include <set>
#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, typename TT, class Compare = std::less<TT>>
class GlobalLockSet {
public:
	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;

	GlobalLockSet()
	: length(0){}
	~GlobalLockSet() {}

	bool add(TT const& item) {
		LockGuard g(m);

		auto iter = data.find(item);
		if(iter != data.end()) {
			return false;
		}
		data.insert(item);

		length.store(data.size(), std::memory_order_relaxed);
		return true;
	}

	bool contains(TT const& item) {
		LockGuard g(m);

		auto iter = data.find(item);
		if(iter == data.end()) {
			return false;
		}

		return true;
	}

	bool remove(TT const& item) {
		LockGuard g(m);

		auto iter = data.find(item);
		if(iter == data.end()) {
			return false;
		}

		data.erase(iter);
		length.store(data.size(), std::memory_order_relaxed);

		return true;
	}

	TT peek_min() {
		LockGuard g(m);

		pheet_assert(size() == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.begin();
		TT ret = *iter;

		return ret;
	}

	TT pop_min() {
		LockGuard g(m);

		pheet_assert(size() == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.begin();
		TT ret = *iter;
		data.erase(iter);
		--length;
		pheet_assert(length == data.size());

		return ret;
	}

	TT peek_max() {
		LockGuard g(m);

		pheet_assert(size() == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.rbegin();
		TT ret = *iter;

		return ret;
	}

	TT pop_max() {
		LockGuard g(m);

		pheet_assert(size() == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.rbegin();
		pheet_assert(iter != data.rend());
		TT ret = *iter;
		data.erase(ret);
		--length;
		pheet_assert(length == data.size());

		return ret;
	}

	inline size_t size() const {
		return length.load(std::memory_order_relaxed);
	}

	static void print_name() {
		std::cout << "GlobalLockSet<";
		Mutex::print_name();
		std::cout << ">";
	}

private:
	std::set<TT, Compare> data;
	std::atomic<size_t> length;
	Mutex m;
};

//template <class Pheet, typename TT, class Compare = std::less<TT>>
//using GlobalLockSetPriorityQueue = PriorityQueueSetWrapper<Pheet, TT, Compare, GlobalLockSet>;

} /* namespace pheet */
#endif /* GLOBALLOCKSET_H_ */
