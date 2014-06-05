/*
 * DummyStrategyStealer.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef DUMMYSTRATEGYSTEALER_H_
#define DUMMYSTRATEGYSTEALER_H_

namespace pheet {

struct DummyStrategyStealerPerformanceCounters {
	static void print_headers() {}
	void print_values() {}
};

template <class Pheet, class TaskStorage>
class DummyStrategyStealer {
public:
	typedef DummyStrategyStealerPerformanceCounters PerformanceCounters;

	DummyStrategyStealer(PerformanceCounters& pc);
	~DummyStrategyStealer();

	typename TaskStorage::T steal_from(typename Pheet::Scheduler::Place* place) {
		return nullable_traits<typename TaskStorage::T>::null_value;
	}
};

template <class Pheet, class TaskStorage>
DummyStrategyStealer<Pheet, TaskStorage>::DummyStrategyStealer(PerformanceCounters& pc) {

}

template <class Pheet, class TaskStorage>
DummyStrategyStealer<Pheet, TaskStorage>::~DummyStrategyStealer() {

}

}

#endif /* DUMMYSTRATEGYSTEALER_H_ */
