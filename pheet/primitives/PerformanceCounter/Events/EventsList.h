/*
 * EventsList.h
 *
 *  Created on: 22 jun 2012
 *      Author: Daniel Cederman
 */

#ifndef EVENTSLIST_H_
#define EVENTSLIST_H_

#include "../../Reducer/List/ListReducer.h"
#include <chrono>

namespace pheet {


template <typename E>
class Event
{
private:
	std::chrono::high_resolution_clock::time_point start;
	E value;

public:
	Event(std::chrono::high_resolution_clock::time_point start, E value)
	:start(start),value(value) {}

	void print(std::chrono::high_resolution_clock::time_point expstart)
    {
		double time = 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(start - expstart).count();
		std::cout << time << ": " << value << std::endl;
    }
};

template <class Pheet, typename E, bool enabled>
class EventsList;


template <class Pheet, typename E>
class EventsList<Pheet, E, true>
{
private:
	ListReducer<Pheet, Event<E> > events;
	std::chrono::high_resolution_clock::time_point start;

public:
	EventsList()
	: start(std::chrono::high_resolution_clock::now()) {}

    inline EventsList(EventsList<Pheet, E, true> const& other)
    :events(other.events) {}

    void add(E const& value)
    {
    	Event<E> e(std::chrono::high_resolution_clock::now(),value);
    	events.add(e);
    }

    void print()
    {
    	std::vector<Event<E> > eventslist = events.get_list();
    	for(size_t i=0; i<eventslist.size(); ++i)
    		eventslist[i].print(start);
    }
};

template <class Pheet, typename E>
class EventsList<Pheet, E, false>
{
public:
	EventsList() {}

	inline EventsList(EventsList<Pheet, E, false> const&) {}

	void add(E const&) {}

	void print() {}
};

template <class Pheet, typename E, bool enabled>
using EventsListPerformanceCounter = EventsList<Pheet, E, enabled>;

}

#endif /* EVENTSLIST_H_ */
