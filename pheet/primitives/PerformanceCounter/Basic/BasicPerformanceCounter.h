/*
 * BasicPerformanceCounter.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICPERFORMANCECOUNTER_H_
#define BASICPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>

#include "../../../settings.h"
#include "../../Reducer/Sum/SumReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, bool> class BasicPerformanceCounter;

template <class Pheet>
class BasicPerformanceCounter<Pheet, false> {
public:
	BasicPerformanceCounter();
	BasicPerformanceCounter(BasicPerformanceCounter<Pheet, false>& other);
	~BasicPerformanceCounter();

	void incr();
	void add(size_t value);
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, false>::BasicPerformanceCounter() {

}

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, false>::BasicPerformanceCounter(BasicPerformanceCounter<Pheet, false>&) {

}

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, false>::~BasicPerformanceCounter() {

}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, false>::incr() {

}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, false>::add(size_t) {

}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, false>::print(char const* const) {

}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, false>::print_header(char const* const) {

}

template <class Pheet>
class BasicPerformanceCounter<Pheet, true> {
public:
	BasicPerformanceCounter();
	BasicPerformanceCounter(BasicPerformanceCounter<Pheet, true>& other);
	~BasicPerformanceCounter();

	void incr();
	void add(size_t value);
	void print(char const* formatting_string);
	static void print_header(char const* const string);
private:
	SumReducer<Pheet, size_t> reducer;
};

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, true>::BasicPerformanceCounter() {

}

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, true>::BasicPerformanceCounter(BasicPerformanceCounter<Pheet, true>& other)
: reducer(other.reducer) {

}

template <class Pheet>
inline
BasicPerformanceCounter<Pheet, true>::~BasicPerformanceCounter() {

}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, true>::incr() {
	reducer.incr();
}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, true>::add(size_t value) {
	reducer.add(value);
}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, true>::print(char const* const formatting_string) {
	printf(formatting_string, reducer.get_sum());
}

template <class Pheet>
inline
void BasicPerformanceCounter<Pheet, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* BASICPERFORMANCECOUNTER_H_ */
