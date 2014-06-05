/*
 * BasicPerformanceCounterVector.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICPERFORMANCECOUNTERVECTOR_H_
#define BASICPERFORMANCECOUNTERVECTOR_H_

#include <stdio.h>
#include <iostream>

#include "../../../settings.h"
#include "../../Reducer/Sum/VectorSumReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, bool> class BasicPerformanceCounterVector;

template <class Pheet>
class BasicPerformanceCounterVector<Pheet, false> {
public:
	BasicPerformanceCounterVector(size_t length);
	BasicPerformanceCounterVector(BasicPerformanceCounterVector<Pheet, false>& other);
	~BasicPerformanceCounterVector();

	void incr(size_t i);
	void print(size_t i, char const* const formatting_string);
	void print_header(size_t i, char const* const string);

//	size_t get_length();
};

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, false>::BasicPerformanceCounterVector(size_t length) {

}

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, false>::BasicPerformanceCounterVector(BasicPerformanceCounterVector<Pheet, false>& other) {

}

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, false>::~BasicPerformanceCounterVector() {

}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, false>::incr(size_t i) {

}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, false>::print(size_t i, char const* const formatting_string) {

}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, false>::print_header(size_t i, char const* const string) {

}
/*
size_t BasicPerformanceCounterVector<Pheet, false>::get_length() {
	return 0;
}*/

template <class Pheet>
class BasicPerformanceCounterVector<Pheet, true> {
public:
	BasicPerformanceCounterVector(size_t length);
	BasicPerformanceCounterVector(BasicPerformanceCounterVector<Pheet, true> & other);
	~BasicPerformanceCounterVector();

	void incr(size_t i);
	void print(size_t i, char const* const formatting_string);
	void print_header(char const* const string);

//	size_t get_length();
private:
	VectorSumReducer<Pheet, size_t> reducer;
};

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, true>::BasicPerformanceCounterVector(size_t length)
: reducer(length) {

}

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, true>::BasicPerformanceCounterVector(BasicPerformanceCounterVector<Pheet, true>& other)
: reducer(other.reducer) {

}

template <class Pheet>
inline
BasicPerformanceCounterVector<Pheet, true>::~BasicPerformanceCounterVector() {

}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, true>::incr(size_t i) {
	reducer.incr(i);
}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, true>::print(size_t i, char const* const formatting_string) {
	size_t const* data = reducer.get_sum();
	printf(formatting_string, data[i]);
}

template <class Pheet>
inline
void BasicPerformanceCounterVector<Pheet, true>::print_header(char const* const string) {
	std::cout << string;
}
/*
size_t BasicPerformanceCounterVector<Pheet, true>::get_length() {
	return reducer.get_length();
}*/

}

#endif /* BASICPERFORMANCECOUNTER_H_ */
