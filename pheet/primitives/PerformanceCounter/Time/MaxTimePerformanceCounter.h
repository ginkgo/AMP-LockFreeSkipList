/*
 * MaxTimePerformanceCounter.h
 *
 *  Created on: 10.09.2013
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MAXTIMEPERFORMANCECOUNTER_H_
#define MAXTIMEPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../../../settings.h"
#include "../../Reducer/Max/MaxReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, bool> class MaxTimePerformanceCounter;

template <class Pheet>
class MaxTimePerformanceCounter<Pheet, false> {
public:
	MaxTimePerformanceCounter();
	MaxTimePerformanceCounter(MaxTimePerformanceCounter<Pheet, false> const& other);
	~MaxTimePerformanceCounter();

	void start_timer();
	void stop_timer();
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, false>::MaxTimePerformanceCounter() {

}

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, false>::MaxTimePerformanceCounter(MaxTimePerformanceCounter<Pheet, false> const&) {

}

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, false>::~MaxTimePerformanceCounter() {

}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, false>::start_timer() {

}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, false>::stop_timer() {

}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, false>::print(char const* const formatting_string) {

}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, false>::print_header(char const* const string) {

}

template <class Pheet>
class MaxTimePerformanceCounter<Pheet, true> {
public:
	MaxTimePerformanceCounter();
	MaxTimePerformanceCounter(MaxTimePerformanceCounter<Pheet, true>& other);
	~MaxTimePerformanceCounter();

	void start_timer();
	void stop_timer();
	void print(char const* formatting_string);
	static void print_header(char const* const string);
private:
	MaxReducer<Pheet, double> reducer;
	using Clock = std::chrono::high_resolution_clock;
	Clock::time_point start_time;
#ifdef PHEET_DEBUG_MODE
	bool is_active;
#endif
};

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, true>::MaxTimePerformanceCounter()
#ifdef PHEET_DEBUG_MODE
: is_active(false)
#endif
{

}

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, true>::MaxTimePerformanceCounter(MaxTimePerformanceCounter<Pheet, true>& other)
: reducer(other.reducer)
#ifdef PHEET_DEBUG_MODE
  , is_active(false)
#endif
{

}

template <class Pheet>
inline
MaxTimePerformanceCounter<Pheet, true>::~MaxTimePerformanceCounter() {

}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, true>::start_timer() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!is_active);
	is_active = true;
#endif
	start_time = Clock::now();
}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, true>::stop_timer() {
	auto stop_time = Clock::now();
	double time = 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
	reducer.add(time);
#ifdef PHEET_DEBUG_MODE
	pheet_assert(is_active);
	is_active = false;
#endif
}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, true>::print(char const* const formatting_string) {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!is_active);
#endif
	printf(formatting_string, reducer.get_max());
}

template <class Pheet>
inline
void MaxTimePerformanceCounter<Pheet, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* TIMEPERFORMANCECOUNTER_H_ */
