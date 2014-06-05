/*
 * BasicScheduler.h
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICSCHEDULER_H_
#define BASICSCHEDULER_H_

#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "../common/FinishRegion.h"
#include "BasicSchedulerPlace.h"
#include "../common/CPUThreadExecutor.h"
#include "../common/DummyBaseStrategy.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"
//#include <pheet/ds/FinishStack/Basic/BasicFinishStack.h>
//#include <pheet/ds/FinishStack/Safer/SaferFinishStack.h>
#include <pheet/ds/FinishStack/MM/MMFinishStack.h>

#include <stdint.h>
#include <limits>

namespace pheet {

template <class Pheet>
struct BasicSchedulerState {
	BasicSchedulerState();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
//	typename Pheet::Scheduler::Task *startup_task;
};

template <class Pheet>
BasicSchedulerState<Pheet>::BasicSchedulerState()
: current_state(0) {

}

/*
 * May only be used once
 */
template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
class BasicSchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
		using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef BasicSchedulerPlace<Pheet, StealingDeque, FinishStack, CallThreshold> Place;
	typedef BasicSchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	typedef DummyBaseStrategy<Pheet> BaseStrategy;

	template <class NP>
	using BT = BasicSchedulerImpl<NP, StealingDeque, FinishStack, CallThreshold>;

	template<uint8_t NewVal>
		using WithCallThreshold = BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, NewVal>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = BasicSchedulerImpl<Pheet, NewTS, FinishStack, CallThreshold>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;
	template <template <class> class NewFS>
		using WithFinishStack = BasicSchedulerImpl<Pheet, StealingDeque, NewFS, CallThreshold>;

	/*
	 * Uses complete machine
	 */
	BasicSchedulerImpl();
	BasicSchedulerImpl(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	BasicSchedulerImpl(procs_t num_places);
	BasicSchedulerImpl(procs_t num_places, PerformanceCounters& performance_counters);
	~BasicSchedulerImpl();

	static void print_name();

	static Place* get_place();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);

	template<class CallTaskType, typename ... TaskParams>
	static void finish(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void finish(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
	static void call(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void call(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
	static void spawn(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
	static void spawn(F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
	static void spawn_prio(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
	static void spawn_prio(Strategy s, F&& f, TaskParams&& ... params);

	static char const name[];
	static procs_t const max_cpus;

private:
	InternalMachineModel machine_model;
	Place** places;
	procs_t num_places;

	State state;

	PerformanceCounters performance_counters;
};

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
char const BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::name[] = "BasicScheduler";

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
procs_t const BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::BasicSchedulerImpl()
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::BasicSchedulerImpl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::BasicSchedulerImpl(procs_t num_places)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::BasicSchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::~BasicSchedulerImpl() {
	delete places[0];
	delete[] places;
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::print_name() {
	std::cout << name;
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
typename BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::Place*
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
procs_t BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
typename BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::Place*
BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::spawn_prio(Strategy, TaskParams&& ... params) {
	spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<class Strategy, typename F, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::spawn_prio(Strategy, F&& f, TaskParams&& ... params) {
	spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class StealingDeque, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void BasicSchedulerImpl<Pheet, StealingDeque, FinishStack, CallThreshold>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

template<class Pheet, typename T>
using BasicSchedulerDefaultStealingDeque = typename Pheet::CDS::template StealingDeque<T>;

template<class Pheet>
using BasicScheduler = BasicSchedulerImpl<Pheet, BasicSchedulerDefaultStealingDeque, MMFinishStack, 3>;

}


#endif /* BASICSCHEDULER_H_ */
