/*
 * SynchroneousScheduler.h
 *
 *  Created on: 06.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SYNCHRONEOUSSCHEDULER_H_
#define SYNCHRONEOUSSCHEDULER_H_

#include "../..//settings.h"
#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "../common/FinishRegion.h"
#include "../common/PlaceBase.h"
#include "../common/DummyBaseStrategy.h"
#include "SynchroneousSchedulerPerformanceCounters.h"

#include <vector>
#include <iostream>

namespace pheet {

template <class Pheet>
class SynchroneousScheduler :public PlaceBase<Pheet> {
public:
	typedef SynchroneousScheduler<Pheet> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
		using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef Self Place;
	typedef FinishRegion<SynchroneousScheduler> Finish;
	typedef SynchroneousSchedulerPerformanceCounters<Pheet> PerformanceCounters;

	typedef DummyBaseStrategy<Pheet> BaseStrategy;

	template <class NP>
	using BT = SynchroneousScheduler<NP>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = SynchroneousScheduler<Pheet>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;

	/*
	 * Uses complete machine
	 */
	SynchroneousScheduler();
	SynchroneousScheduler(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	SynchroneousScheduler(procs_t num_places);
	SynchroneousScheduler(procs_t num_places, PerformanceCounters& performance_counters);
	~SynchroneousScheduler();

	static void print_name();

	static Self* get();
	static Place* get_place();
	static procs_t get_id();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void finish(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void spawn(F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_prio(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_prio(Strategy s, F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_s(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy s, F&& f, TaskParams&& ... params);

	procs_t get_distance(Self* other) {
		pheet_assert(other == this);
		return 0;
	}

	void start_finish_region() {}
	void end_finish_region() {}

	static char const name[];
	static procs_t const max_cpus;

private:
	PerformanceCounters pc;

	Self* parent_place;

	static THREAD_LOCAL Self* local_place;
};

template <class Pheet>
char const SynchroneousScheduler<Pheet>::name[] = "SynchroneousScheduler";

template <class Pheet>
procs_t const SynchroneousScheduler<Pheet>::max_cpus = 1;

template <class Pheet>
THREAD_LOCAL SynchroneousScheduler<Pheet> *
SynchroneousScheduler<Pheet>::local_place = nullptr;

template <class Pheet>
SynchroneousScheduler<Pheet>::SynchroneousScheduler()
: parent_place(nullptr) {
	local_place = this;
}

template <class Pheet>
SynchroneousScheduler<Pheet>::SynchroneousScheduler(PerformanceCounters& performance_counters)
: pc(performance_counters), parent_place(nullptr){
	local_place = this;
}

template <class Pheet>
SynchroneousScheduler<Pheet>::SynchroneousScheduler(procs_t num_places)
: parent_place(nullptr) {
	pheet_assert(num_places == 1);
	local_place = this;
}

template <class Pheet>
SynchroneousScheduler<Pheet>::SynchroneousScheduler(procs_t num_places, PerformanceCounters& performance_counters)
: pc(performance_counters), parent_place(nullptr) {
	pheet_assert(num_places == 1);
	local_place = this;
}

template <class Pheet>
SynchroneousScheduler<Pheet>::~SynchroneousScheduler() {
	local_place = parent_place;
}

template <class Pheet>
void SynchroneousScheduler<Pheet>::print_name() {
	std::cout << name;
}

template <class Pheet>
template<class CallTaskType, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::finish(TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<typename F, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::finish(F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet>
template<class CallTaskType, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn(TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<typename F, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn(F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}
/*
template <class Pheet>
template<class CallTaskType, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::call(TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<typename F, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::call(F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}*/

template <class Pheet>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn_prio(Strategy, TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<class Strategy, typename F, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn_prio(Strategy, F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn_s(Strategy, TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<class Strategy, typename F, typename ... TaskParams>
void SynchroneousScheduler<Pheet>::spawn_s(Strategy, F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet>
SynchroneousScheduler<Pheet>* SynchroneousScheduler<Pheet>::get() {
	return local_place;
}

template <class Pheet>
SynchroneousScheduler<Pheet>* SynchroneousScheduler<Pheet>::get_place() {
	return local_place;
}

template <class Pheet>
procs_t SynchroneousScheduler<Pheet>::get_id() {
	return 0;
}

template <class Pheet>
procs_t SynchroneousScheduler<Pheet>::get_place_id() {
	return 0;
}

template <class Pheet>
SynchroneousScheduler<Pheet>* SynchroneousScheduler<Pheet>::get_place_at(procs_t place_id) {
	pheet_assert(place_id == 0);
	return local_place;
}


}

#endif /* SYNCHRONEOUSSCHEDULER_H_ */
