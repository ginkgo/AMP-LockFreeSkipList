/*
 * MixedModeScheduler.h
 *
 *  Created on: 16.06.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICMIXEDMODESCHEDULER_H_
#define BASICMIXEDMODESCHEDULER_H_

#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "MixedModeSchedulerPlace.h"
#include "../common/CPUThreadExecutor.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"

#include <stdint.h>
#include <limits>
#include <vector>
#include <iostream>

/*
 *
 */
namespace pheet {

template <class Pheet>
struct MixedModeSchedulerState {
	MixedModeSchedulerState();

//	procs_t team_size;
	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
//	Task *startup_task;
};

template <class Pheet>
MixedModeSchedulerState<Pheet>::MixedModeSchedulerState()
: current_state(0) {

}

/*
 * May only be used once
 */
template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
class MixedModeSchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef typename Pheet::Barrier Barrier;
	typedef MixedModeSchedulerImpl<Pheet, StealingDeque> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
		using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef MixedModeSchedulerPlace<Pheet, StealingDeque> Place;
	typedef MixedModeSchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	template <class NP>
	using BT = MixedModeSchedulerImpl<NP, StealingDeque>;

	template <template <class, typename> class NewTS>
		using WithTaskStorage = MixedModeSchedulerImpl<Pheet, NewTS>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;

	/*
	 * Uses complete machine
	 */
	MixedModeSchedulerImpl();
	MixedModeSchedulerImpl(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	MixedModeSchedulerImpl(procs_t num_places);
	MixedModeSchedulerImpl(procs_t num_places, PerformanceCounters& performance_counters);
	~MixedModeSchedulerImpl();

	static void print_name();

	static Place* get_place();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);

	template<class CallTaskType, typename ... TaskParams>
	static void finish(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void finish(F&& f, TaskParams&& ... params);
/*
	template<class CallTaskType, typename ... TaskParams>
	void finish_nt(procs_t nt, TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	void finish_nt(procs_t nt, F&& f, TaskParams&& ... params);
*/
	template<class CallTaskType, typename ... TaskParams>
	static void call(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void call(F&& f, TaskParams&& ... params);
/*
	template<class CallTaskType, typename ... TaskParams>
	void call_nt(procs_t nt, TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	void call_nt(procs_t nt, F&& f, TaskParams&& ... params);
*/
	template<class CallTaskType, typename ... TaskParams>
	static void spawn(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void spawn(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
	static void spawn_nt(procs_t nt, TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void spawn_nt(procs_t nt, F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
	static void spawn_prio(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
	static void spawn_prio(Strategy s, F&& f, TaskParams&& ... params);

	procs_t get_max_team_size();

	static char const name[];
	static procs_t const max_cpus;
private:
	InternalMachineModel machine_model;
	Place** places;
	procs_t num_places;

	State state;

	PerformanceCounters performance_counters;
};

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
char const MixedModeSchedulerImpl<Pheet, StealingDeque>::name[] = "MixedModeScheduler";

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
procs_t const MixedModeSchedulerImpl<Pheet, StealingDeque>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
MixedModeSchedulerImpl<Pheet, StealingDeque>::MixedModeSchedulerImpl()
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
MixedModeSchedulerImpl<Pheet, StealingDeque>::MixedModeSchedulerImpl(PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
MixedModeSchedulerImpl<Pheet, StealingDeque>::MixedModeSchedulerImpl(procs_t num_places)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
MixedModeSchedulerImpl<Pheet, StealingDeque>::MixedModeSchedulerImpl(procs_t num_places, PerformanceCounters& performance_counters)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
MixedModeSchedulerImpl<Pheet, StealingDeque>::~MixedModeSchedulerImpl() {
	delete places[0];
	delete[] places;
}

/*
template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::initialize_tecs(InternalMachineModel* ch, size_t offset, std::vector<typename Place::LevelDescription*>* levels) {
	if(ch->get_size() > 1) {
		std::vector<InternalMachineModel*> const* sub = ch->get_subsets();

		if(sub->size() == 2) {
			InternalMachineModel* sub0 = (*sub)[0];
			InternalMachineModel* sub1 = (*sub)[1];

			typename Place::LevelDescription ld;
			ld.total_size = ch->get_size();
			ld.local_id = 0;
			ld.num_partners = sub1->get_size();
			ld.partners = places + offset + sub0->get_size();
			ld.reverse_ids = false;

			levels->push_back(&ld);
			initialize_tecs(sub0, offset, levels);
			ld.local_id = sub0->get_size();
			ld.num_partners = ld.local_id;
			ld.partners = places + offset;
			ld.reverse_ids = true;
			initialize_tecs(sub1, offset + ld.local_id, levels);

			levels->pop_back();
		}
		else {
			InternalMachineModel* sub0 = (*sub)[0];

			initialize_tecs(sub0, offset, levels);
		}
	}
	else {
		typename Place::LevelDescription ld;
		ld.total_size = 1;
		ld.local_id = 0;
		ld.num_partners = 0;
		ld.partners = NULL;
		ld.reverse_ids = false;
		levels->push_back(&ld);
		Place *tec = new Place(levels, ch->get_cpus(), &state, performance_counters);
		places[offset] = tec;
		levels->pop_back();
	}
}*/


template <class Pheet, template <class P, typename T> class StealingDeque>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::print_name() {
	std::cout << name;
}

template <class Pheet, template <class P, typename T> class StealingDeque>
typename MixedModeSchedulerImpl<Pheet, StealingDeque>::Place*
MixedModeSchedulerImpl<Pheet, StealingDeque>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T> class StealingDeque>
procs_t MixedModeSchedulerImpl<Pheet, StealingDeque>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T> class StealingDeque>
typename MixedModeSchedulerImpl<Pheet, StealingDeque>::Place*
MixedModeSchedulerImpl<Pheet, StealingDeque>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish_nt<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<typename F, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish_nt(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<typename F, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn_nt(procs_t nt, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn_nt<CallTaskType>(nt, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<typename F, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn_nt(procs_t nt, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_nt(nt, f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn_prio(Strategy s, TaskParams&& ... params) {
	spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class Strategy, typename F, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::spawn_prio(Strategy s, F&& f, TaskParams&& ... params) {
	spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque>
template<typename F, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

/*
template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::finish(TaskParams&& ... params) {
	finish_nt<CallTaskType, TaskParams ...>((procs_t)1, std::forward<TaskParams&&>(params) ...);
}
*/
/*
template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerImpl<Pheet, StealingDeque>::finish_nt(procs_t nt, TaskParams&& ... params) {
	CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
	state.team_size = nt;
	state.startup_task = task;
	state.current_state = 1;

	// signal scheduler places that execution may start
	state.state_barrier.signal(0);

	for(procs_t i = 0; i < num_places; i++) {
		places[i]->join();
		delete places[i];
	}
	delete[] places;
}
*/

template <class Pheet, template <class Scheduler, typename T> class StealingDeque>
procs_t MixedModeSchedulerImpl<Pheet, StealingDeque>::get_max_team_size() {
	return num_places;
}

template<class Pheet, typename T>
using MixedModeSchedulerDefaultStealingDeque = typename Pheet::CDS::template StealingDeque<T>;

template <class Pheet>
using MixedModeScheduler = MixedModeSchedulerImpl<Pheet, MixedModeSchedulerDefaultStealingDeque>;

}

#endif /* BASICMIXEDMODESCHEDULER_H_ */
