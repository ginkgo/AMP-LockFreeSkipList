/*
 * ListPerformanceCounter.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LISTPERFORMANCECOUNTER_H_
#define LISTPERFORMANCECOUNTER_H_

#include <pheet/primitives/Reducer/List/ListReducer.h>

namespace pheet {

template <class Pheet, typename E, bool enabled>
class ListPerformanceCounter;

template <class Pheet, typename E>
class ListPerformanceCounter<Pheet, E, true> {
public:
	typedef ListPerformanceCounter<Pheet, E, true> Self;

	ListPerformanceCounter() {}
	ListPerformanceCounter(Self& other)
	:reducer(other.reducer) {}
	~ListPerformanceCounter() {}

	void add(E const& item) {
		reducer.add(item);
	}
	void print(char const* const prefix, char const* const postfix) {
		std::cout << prefix;
		auto end = reducer.get_list().end();
		for(auto i = reducer.get_list().begin(); i != end; ++i) {
			i->print();
		}
		std::cout << postfix;
	}
	static void print_header(char const* const string) {
		std::cout << string;
	}

private:
	ListReducer<Pheet, E> reducer;
};



template <class Pheet, typename E>
class ListPerformanceCounter<Pheet, E, false> {
public:
	typedef ListPerformanceCounter<Pheet, E, false> Self;

	ListPerformanceCounter() {}
	ListPerformanceCounter(Self&) {}
	~ListPerformanceCounter() {}

	void add(E const&) {}
	void print(char const* const, char const* const) {}
	static void print_header(char const* const) {}
};

} /* namespace pheet */
#endif /* LISTPERFORMANCECOUNTER_H_ */
