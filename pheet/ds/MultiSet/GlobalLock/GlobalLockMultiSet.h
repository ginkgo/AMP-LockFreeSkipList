/*
 * GlobalLockMultiSet.h
 *
 *  Created on: May 30, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef GLOBALLOCKMULTISET_H_
#define GLOBALLOCKMULTISET_H_

#include <set>
#include <iostream>
#include "../../../misc/type_traits.h"
#include "../../PriorityQueue/MultiSetWrapper/PriorityQueueMultiSetWrapper.h"

namespace pheet {

template <class Pheet, typename TT, class Compare = std::less<TT>>
class GlobalLockMultiSet {
public:
	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;

	GlobalLockMultiSet()
	: length(0){}
	~GlobalLockMultiSet() {}

	void put(TT const& item) {
		LockGuard g(m);

		data.insert(item);

		length = data.size();
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
		length = data.size();

		return true;
	}

	TT peek_min() {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.begin();
		TT ret = *iter;

		return ret;
	}

	TT pop_min() {
		LockGuard g(m);

		pheet_assert(length == data.size());
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

		pheet_assert(length == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.rbegin();
		TT ret = *iter;

		return ret;
	}

	TT pop_max() {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.rbegin();
		pheet_assert(iter != data.rend());
		TT ret = *iter;
		data.erase(--(iter.base()));
		--length;
		pheet_assert(length == data.size());

		return ret;
	}

	inline size_t get_length() const {
		return length;
	}

	inline size_t size() const {
		return get_length();
	}

	static void print_name() {
		std::cout << "GlobalLockMultiSet<";
		Mutex::print_name();
		std::cout << ">";
	}

private:
	std::multiset<TT, Compare> data;
	size_t length;
	Mutex m;
};

template <class Pheet, typename TT, class Compare = std::less<TT>>
using GlobalLockMultiSetPriorityQueue = PriorityQueueMultiSetWrapper<Pheet, TT, Compare, GlobalLockMultiSet>;

} /* namespace pheet */
#endif /* GLOBALLOCKMULTISET_H_ */
