/*
 * GlobalLockMap.h
 *
 *  Created on: May 30, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef GLOBALLOCKMAP_H_
#define GLOBALLOCKMAP_H_

#include <map>
#include <iostream>
#include "../../../misc/type_traits.h"

namespace pheet {

template <class Pheet, typename Key, typename TT, class Compare = std::less<Key>>
class GlobalLockMap {
public:
	typedef typename Pheet::Mutex Mutex;
	typedef typename Pheet::LockGuard LockGuard;

	GlobalLockMap()
	: length(0){}
	~GlobalLockMap() {}

	void put(Key const& key, TT const& item) {
		LockGuard g(m);

		data[key] = item;

		length = data.size();
	}

	TT get(Key const& key) {
		LockGuard g(m);

		auto iter = data.find(key);
		if(iter == data.end()) {
			return nullable_traits<TT>::null_value;
		}

		TT ret = *iter;

		return ret;
	}

	TT remove(Key const& key) {
		LockGuard g(m);

		auto iter = data.find(key);
		if(iter == data.end()) {
			return nullable_traits<TT>::null_value;
		}

		TT ret = *iter;
		data.erase(iter);

		return ret;
	}

	TT peek_min() {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.begin();
		TT ret = iter->second;

		return ret;
	}

	TT pop_min() {
		LockGuard g(m);

		pheet_assert(length == data.size());
		if(data.empty()) {
			return nullable_traits<TT>::null_value;
		}

		auto iter = data.begin();
		TT ret = iter->second;
		data.erase(iter);

		return ret;
	}

	inline size_t get_length() const {
		return length;
	}

	inline size_t size() const {
		return get_length();
	}

	static void print_name() {
		std::cout << "GlobalLockMap<";
		Mutex::print_name();
		std::cout << ">";
	}

private:
	std::map<Key, TT, Compare> data;
	size_t length;
	Mutex m;
};

} /* namespace pheet */
#endif /* GLOBALLOCKMAP_H_ */
