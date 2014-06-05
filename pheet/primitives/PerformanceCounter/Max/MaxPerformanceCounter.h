/*
 * MaxPerformanceCounter.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MAXPERFORMANCECOUNTER_H_
#define MAXPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>

#include "../../../settings.h"
#include "../../Reducer/Max/MaxReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, typename T, bool> class MaxPerformanceCounter;

template <class Pheet, typename T>
class MaxPerformanceCounter<Pheet, T, false> {
public:
	MaxPerformanceCounter();
	MaxPerformanceCounter(MaxPerformanceCounter<Pheet, T, false> const& other);
	~MaxPerformanceCounter();

	void add_value(T const&);
	void add(T const&) {}

	void incr() {}
	void add_counted() {}

	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet, typename T>
inline MaxPerformanceCounter<Pheet, T, false>::MaxPerformanceCounter() {

}

template <class Pheet, typename T>
inline MaxPerformanceCounter<Pheet, T, false>::MaxPerformanceCounter(MaxPerformanceCounter<Pheet, T, false> const&) {

}

template <class Pheet, typename T>
inline
MaxPerformanceCounter<Pheet, T, false>::~MaxPerformanceCounter() {

}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, false>::add_value(T const&) {

}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, false>::print(char const* const) {

}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, false>::print_header(char const* const) {

}

template <class Pheet, typename T>
class MaxPerformanceCounter<Pheet, T, true> {
public:
	MaxPerformanceCounter();
	MaxPerformanceCounter(MaxPerformanceCounter<Pheet, T, true>& other);
	~MaxPerformanceCounter();

	void add_value(T const& value);
	void add(T const& value) {
		add_value(value);
	}
	void incr() {
		++value;
	}
	void add_counted() {
		add(value);
		value = 0;
	}
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
private:
	MaxReducer<Pheet, T> reducer;
	T value;
};

template <class Pheet, typename T>
inline
MaxPerformanceCounter<Pheet, T, true>::MaxPerformanceCounter()
:value(0) {

}

template <class Pheet, typename T>
inline
MaxPerformanceCounter<Pheet, T, true>::MaxPerformanceCounter(MaxPerformanceCounter<Pheet, T, true>& other)
: reducer(other.reducer), value(0) {

}

template <class Pheet, typename T>
inline
MaxPerformanceCounter<Pheet, T, true>::~MaxPerformanceCounter() {

}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, true>::add_value(T const& value) {
	reducer.add_value(value);
}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, true>::print(char const* const formatting_string) {
	printf(formatting_string, reducer.get_max());
}

template <class Pheet, typename T>
inline
void MaxPerformanceCounter<Pheet, T, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* MAXPERFORMANCECOUNTER_H_ */
