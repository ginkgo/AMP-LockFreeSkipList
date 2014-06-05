/*
 * SORPerformanceCounters.h
 *
 *  Created on: 21 jun 2012
 *      Author: Daniel Cederman
 */

#ifndef SORPERFORMANCECOUNTERS_H_
#define SORPERFORMANCECOUNTERS_H_

#include "pheet/pheet.h"
#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include "pheet/primitives/PerformanceCounter/Events/EventsList.h"

namespace pheet {

 template <class Pheet>
class SORPerformanceCounters {
public:
	SORPerformanceCounters();
	SORPerformanceCounters(SORPerformanceCounters& other);
	SORPerformanceCounters(SORPerformanceCounters&& other);
	~SORPerformanceCounters();

	static void print_headers();
	void print_values();

	BasicPerformanceCounter<Pheet, sor_slices_rescheduled_at_same_place> slices_rescheduled_at_same_place;
	BasicPerformanceCounter<Pheet, sor_average_distance> average_distance;
//	EventsList<Pheet, size_t, sor_events> events;
};


template <class Pheet>
inline SORPerformanceCounters<Pheet>::SORPerformanceCounters()
{

}

template <class Pheet>
inline SORPerformanceCounters<Pheet>::SORPerformanceCounters(SORPerformanceCounters<Pheet>& other)
  :slices_rescheduled_at_same_place(other.slices_rescheduled_at_same_place),
   average_distance(other.average_distance)////,events(other.events)
{

}

template <class Pheet>
inline SORPerformanceCounters<Pheet>::SORPerformanceCounters(SORPerformanceCounters<Pheet>&& other)
  :slices_rescheduled_at_same_place(other.slices_rescheduled_at_same_place),
   average_distance(other.average_distance)//,events(other.events)
{

}

template <class Pheet>
inline SORPerformanceCounters<Pheet>::~SORPerformanceCounters() {

}

template <class Pheet>
inline void SORPerformanceCounters<Pheet>::print_headers() {
	BasicPerformanceCounter<Pheet, sor_slices_rescheduled_at_same_place>::print_header("slices_rescheduled_at_same_place\t");
	BasicPerformanceCounter<Pheet, sor_average_distance>::print_header("average_distance\t");

}

template <class Pheet>
inline void SORPerformanceCounters<Pheet>::print_values() {
	slices_rescheduled_at_same_place.print("%d\t");
    average_distance.print("%d\t");
	//events.print();

}

}



#endif /* SORPERFORMANCECOUNTERS_H_ */
