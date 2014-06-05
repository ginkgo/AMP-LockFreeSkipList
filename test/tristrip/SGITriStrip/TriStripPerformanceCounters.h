/*
 * TriStripPerformanceCounters.h
 *
 *  Created on: 21 jun 2012
 *      Author: Daniel Cederman
 */

#ifndef TriStripPERFORMANCECOUNTERS_H_
#define TriStripPERFORMANCECOUNTERS_H_

//#include "pheet/pheet.h"
//#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
//#include "pheet/primitives/PerformanceCounter/Events/EventsList.h"

namespace pheet {

 template <class Pheet>
class TriStripPerformanceCounters {
public:
	TriStripPerformanceCounters();
	TriStripPerformanceCounters(TriStripPerformanceCounters& other);
	TriStripPerformanceCounters(TriStripPerformanceCounters&& other);
	~TriStripPerformanceCounters();

	static void print_headers();
	void print_values();

	//	BasicPerformanceCounter<Pheet, true> tristripcount;
	SumReducer<Pheet, size_t> tristripcount;
	SumReducer<Pheet, size_t> nodecount;


	//	BasicPerformanceCounter<Pheet, sor_slices_rescheduled_at_same_place> slices_rescheduled_at_same_place;
	//	BasicPerformanceCounter<Pheet, sor_average_distance> average_distance;
//	EventsList<Pheet, size_t, sor_events> events;

	void addstrip(std::vector<GraphNode*> strip)
        {
              nodecount.add(strip.size());
	  //              printf("%d\n",strip.size());
	  // Should store the strip, but for now only count it
              tristripcount.incr();
        }

        size_t getNodeCount()
        {
              return nodecount.get_sum();
        }

        size_t getCount()
        {
              return tristripcount.get_sum();
        }
        //      BasicPerformanceCounter<Pheet, true> tristripcount;


};


template <class Pheet>
inline TriStripPerformanceCounters<Pheet>::TriStripPerformanceCounters()
{

}

template <class Pheet>
inline TriStripPerformanceCounters<Pheet>::TriStripPerformanceCounters(TriStripPerformanceCounters<Pheet>& other)
  :tristripcount(other.tristripcount),nodecount(other.nodecount)
  // :slices_rescheduled_at_same_place(other.slices_rescheduled_at_same_place),
  // average_distance(other.average_distance)////,events(other.events)
{

}

template <class Pheet>
inline TriStripPerformanceCounters<Pheet>::TriStripPerformanceCounters(TriStripPerformanceCounters<Pheet>&& other)
  :tristripcount(other.tristripcount),nodecount(other.nodecount)
  // :slices_rescheduled_at_same_place(other.slices_rescheduled_at_same_place),
  // average_distance(other.average_distance)//,events(other.events)
{

}

template <class Pheet>
inline TriStripPerformanceCounters<Pheet>::~TriStripPerformanceCounters() {

}

template <class Pheet>
inline void TriStripPerformanceCounters<Pheet>::print_headers() {
//	BasicPerformanceCounter<Pheet, sor_slices_rescheduled_at_same_place>::print_header("slices_rescheduled_at_same_place\t");
//	BasicPerformanceCounter<Pheet, sor_average_distance>::print_header("average_distance\t");

}

template <class Pheet>
inline void TriStripPerformanceCounters<Pheet>::print_values() {
//	slices_rescheduled_at_same_place.print("%d\t");
 //   average_distance.print("%d\t");
	//events.print();

}

}



#endif /* TriStripPERFORMANCECOUNTERS_H_ */
