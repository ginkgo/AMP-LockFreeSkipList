/*
 * CentralizedScheduler.h
 *
 *  Created on: May 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef CENTRALIZEDSCHEDULER_H_
#define CENTRALIZEDSCHEDULER_H_

#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "../common/FinishRegion.h"
#include "CentralizedSchedulerPlace.h"
#include "../common/CPUThreadExecutor.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"

#include <pheet/ds/FinishStack/MM/MMFinishStack.h>

#include <stdint.h>
#include <limits>

namespace pheet {

template <class Pheet>
struct CentralizedSchedulerState {
	CentralizedSchedulerState();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
//	typename Pheet::Scheduler::Task *startup_task;
};

template <class Pheet>
CentralizedSchedulerState<Pheet>::CentralizedSchedulerState()
: current_state(0) {

}

/*
 * May only be used once
 */
template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
class CentralizedSchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
		using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef CentralizedSchedulerPlace<Pheet, TaskStorageT, FinishStack, CallThreshold> Place;
	typedef CentralizedSchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	typedef typename Place::TaskStorage TaskStorage;

	template <class NP>
	using BT = CentralizedSchedulerImpl<NP, TaskStorageT, FinishStack, CallThreshold>;

	template<uint8_t NewVal>
		using WithCallThreshold = CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, NewVal>;
	template <template <class, typename> class NewTS>
	using WithTaskStorage = CentralizedSchedulerImpl<Pheet, NewTS, FinishStack, CallThreshold>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;
	template <template <class> class NewFS>
		using WithFinishStack = CentralizedSchedulerImpl<Pheet, TaskStorageT, NewFS, CallThreshold>;

	/*
	 * Uses complete machine
	 */
	CentralizedSchedulerImpl();
	CentralizedSchedulerImpl(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	CentralizedSchedulerImpl(procs_t num_places);
	CentralizedSchedulerImpl(procs_t num_places, PerformanceCounters& performance_counters);
	~CentralizedSchedulerImpl();

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

	TaskStorage task_storage;

	State state;

	PerformanceCounters performance_counters;
};

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
char const CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::name[] = "CentralizedScheduler";

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
procs_t const CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::CentralizedSchedulerImpl()
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::CentralizedSchedulerImpl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::CentralizedSchedulerImpl(procs_t num_places)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::CentralizedSchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::~CentralizedSchedulerImpl() {
	delete places[0];
	delete[] places;
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::print_name() {
	std::cout << name  << "<";
	Place::TaskStorage::print_name();
	std::cout << ", " << (int)CallThreshold << ">";
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
typename CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::Place*
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
procs_t CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
typename CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::Place*
CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::spawn_prio(Strategy s, TaskParams&& ... params) {
	spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<class Strategy, typename F, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::spawn_prio(Strategy s, F&& f, TaskParams&& ... params) {
	spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedSchedulerImpl<Pheet, TaskStorageT, FinishStack, CallThreshold>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

template<class Pheet, typename T>
using CentralizedSchedulerDefaultTaskStorageT = typename Pheet::CDS::template Stack<T>;

template<class Pheet>
using CentralizedScheduler = CentralizedSchedulerImpl<Pheet, CentralizedSchedulerDefaultTaskStorageT, MMFinishStack, 3>;

}

#endif /* CENTRALIZEDSCHEDULER_H_ */
