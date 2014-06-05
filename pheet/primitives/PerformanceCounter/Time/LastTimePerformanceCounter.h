/*
 * LastTimePerformanceCounter.h
 *
 *  Created on: 10.08.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef LASTTIMEPERFORMANCECOUNTER_H_
#define LASTTIMEPERFORMANCECOUNTER_H_

#include <stdio.h>
#include <iostream>
#include <chrono>

#include "../../../settings.h"
#include "../../Reducer/Max/MaxReducer.h"

/*
 *
 */
namespace pheet {

template <class Pheet, bool> class LastTimePerformanceCounter;

template <class Pheet>
class LastTimePerformanceCounter<Pheet, false> {
public:
	LastTimePerformanceCounter();
	LastTimePerformanceCounter(LastTimePerformanceCounter<Pheet, false> const& other);
	~LastTimePerformanceCounter();

	inline void start_timer() {}
	inline void take_time() {}
	void print(char const* const formatting_string);
	static void print_header(char const* const string);
};

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, false>::LastTimePerformanceCounter() {

}

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, false>::LastTimePerformanceCounter(LastTimePerformanceCounter<Pheet, false> const& other) {

}

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, false>::~LastTimePerformanceCounter() {

}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, false>::print(char const* const formatting_string) {

}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, false>::print_header(char const* const string) {

}

template <class Pheet>
class LastTimePerformanceCounter<Pheet, true> {
public:
	LastTimePerformanceCounter();
	LastTimePerformanceCounter(LastTimePerformanceCounter<Pheet, true>& other);
	~LastTimePerformanceCounter();

	void start_timer();
	void take_time();
	void print(char const* formatting_string);
	static void print_header(char const* const string);
private:

	MaxReducer<Pheet, double> reducer;
	std::chrono::high_resolution_clock::time_point start_time;
};

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, true>::LastTimePerformanceCounter()
{
	start_timer();
}

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, true>::LastTimePerformanceCounter(LastTimePerformanceCounter<Pheet, true>& other)
: reducer(other.reducer), start_time(other.start_time)
{

}

template <class Pheet>
inline
LastTimePerformanceCounter<Pheet, true>::~LastTimePerformanceCounter() {

}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, true>::start_timer() {
	start_time = std::chrono::high_resolution_clock::now();
}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, true>::take_time() {
	auto stop_time = std::chrono::high_resolution_clock::now();
	double time = 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
	reducer.add(time);
}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, true>::print(char const* const formatting_string) {
	printf(formatting_string, reducer.get_max());
}

template <class Pheet>
inline
void LastTimePerformanceCounter<Pheet, true>::print_header(char const* const string) {
	std::cout << string;
}

}

#endif /* LASTTIMEPERFORMANCECOUNTER_H_ */
