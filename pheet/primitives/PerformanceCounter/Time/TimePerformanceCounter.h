/*
 * TimePerformanceCounter.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TIMEPERFORMANCECOUNTER_H_
#define TIMEPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../../../settings.h"
#include "../../Reducer/Sum/SumReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, bool> class TimePerformanceCounter;

template <class Pheet>
class TimePerformanceCounter<Pheet, false> {
public:
	TimePerformanceCounter();
	TimePerformanceCounter(TimePerformanceCounter<Pheet, false> const& other);
	~TimePerformanceCounter();

	void start_timer();
	void stop_timer();
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet>
inline
TimePerformanceCounter<Pheet, false>::TimePerformanceCounter() {

}

template <class Pheet>
inline
TimePerformanceCounter<Pheet, false>::TimePerformanceCounter(TimePerformanceCounter<Pheet, false> const&) {

}

template <class Pheet>
inline
TimePerformanceCounter<Pheet, false>::~TimePerformanceCounter() {

}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, false>::start_timer() {

}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, false>::stop_timer() {

}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, false>::print(char const* const) {

}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, false>::print_header(char const* const) {

}

template <class Pheet>
class TimePerformanceCounter<Pheet, true> {
public:
	TimePerformanceCounter();
	TimePerformanceCounter(TimePerformanceCounter<Pheet, true>& other);
	~TimePerformanceCounter();

	void start_timer();
	void stop_timer();
	void print(char const* formatting_string);
	static void print_header(char const* const string);
private:
	SumReducer<Pheet, double> reducer;
	std::chrono::high_resolution_clock::time_point start_time;
#ifdef PHEET_DEBUG_MODE
	bool is_active;
#endif
};

template <class Pheet>
inline
TimePerformanceCounter<Pheet, true>::TimePerformanceCounter()
#ifdef PHEET_DEBUG_MODE
: is_active(false)
#endif
{

}

template <class Pheet>
inline
TimePerformanceCounter<Pheet, true>::TimePerformanceCounter(TimePerformanceCounter<Pheet, true>& other)
: reducer(other.reducer)
#ifdef PHEET_DEBUG_MODE
  , is_active(false)
#endif
{

}

template <class Pheet>
inline
TimePerformanceCounter<Pheet, true>::~TimePerformanceCounter() {

}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, true>::start_timer() {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!is_active);
	is_active = true;
#endif
	start_time = std::chrono::high_resolution_clock::now();
}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, true>::stop_timer() {
	auto stop_time = std::chrono::high_resolution_clock::now();
	double time = 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
	reducer.add(time);
#ifdef PHEET_DEBUG_MODE
	pheet_assert(is_active);
	is_active = false;
#endif
}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, true>::print(char const* const formatting_string) {
#ifdef PHEET_DEBUG_MODE
	pheet_assert(!is_active);
#endif
	printf(formatting_string, reducer.get_sum());
}

template <class Pheet>
inline
void TimePerformanceCounter<Pheet, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* TIMEPERFORMANCECOUNTER_H_ */
