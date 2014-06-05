/*
 * MinPerformanceCounter.h
 *
 *  Created on: Nov 8, 2011
 *      Author: Martin Wimmer
 */

#ifndef MINPERFORMANCECOUNTER_H_
#define MINPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>

#include "../../../settings.h"
#include "../../Reducer/Min/MinReducer.h"

namespace pheet {

template <class Pheet, typename T, bool> class MinPerformanceCounter;

template <class Pheet, typename T>
class MinPerformanceCounter<Pheet, T, false> {
public:
	MinPerformanceCounter();
	MinPerformanceCounter(MinPerformanceCounter<Pheet, T, false> const& other);
	~MinPerformanceCounter();

	void add_value(size_t value);
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, false>::MinPerformanceCounter() {

}

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, false>::MinPerformanceCounter(MinPerformanceCounter<Pheet, T, false> const&) {

}

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, false>::~MinPerformanceCounter() {

}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, false>::add_value(size_t) {

}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, false>::print(char const* const) {

}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, false>::print_header(char const* const) {

}

template <class Pheet, typename T>
class MinPerformanceCounter<Pheet, T, true> {
public:
	MinPerformanceCounter();
	MinPerformanceCounter(MinPerformanceCounter<Pheet, T, true>& other);
	~MinPerformanceCounter();

	void add_value(size_t value);
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
private:
	MinReducer<Pheet, size_t> reducer;
};

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, true>::MinPerformanceCounter() {

}

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, true>::MinPerformanceCounter(MinPerformanceCounter<Pheet, T, true>& other)
: reducer(other.reducer) {

}

template <class Pheet, typename T>
inline
MinPerformanceCounter<Pheet, T, true>::~MinPerformanceCounter() {

}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, true>::add_value(size_t value) {
	reducer.add_value(value);
}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, true>::print(char const* const formatting_string) {
	printf(formatting_string, reducer.get_min());
}

template <class Pheet, typename T>
inline
void MinPerformanceCounter<Pheet, T, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* MINPERFORMANCECOUNTER_H_ */
