/*
 * CentralizedPriorityScheduler.h
 *
 *  Created on: 19.09.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CENTRALIZEDPRIORITYSCHEDULER_H_
#define CENTRALIZEDPRIORITYSCHEDULER_H_

#include "../../settings.h"
#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "../common/FinishRegion.h"
#include "CentralizedPrioritySchedulerPlace.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"
//#include "../../ds/PriorityTaskStorage/Modular/ModularTaskStorage.h"
#include "../../ds/PriorityQueue/GlobalLockHeap/GlobalLockHeap.h"
#include "../strategies/LifoFifo/LifoFifoStrategy.h"

#include <pheet/ds/FinishStack/MM/MMFinishStack.h>

#include <stdint.h>
#include <limits>

namespace pheet {

template <class Pheet>
struct CentralizedPrioritySchedulerState {
	CentralizedPrioritySchedulerState();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
	typename Pheet::Scheduler::Task* startup_task;
};

template <class Pheet>
CentralizedPrioritySchedulerState<Pheet>::CentralizedPrioritySchedulerState()
: current_state(0), startup_task(NULL) {

}


template <class Pheet>
struct CentralizedPrioritySchedulerTaskStorageItem {
	typedef typename Pheet::Environment::Place Place;

	CentralizedPrioritySchedulerTaskStorageItem();

	typename Pheet::Environment::Place::Task* task;
	typename Pheet::Environment::Place::StackElement* stack_element;
	prio_t priority;

	bool operator==(CentralizedPrioritySchedulerTaskStorageItem<Pheet> const& other) const;
	bool operator!=(CentralizedPrioritySchedulerTaskStorageItem<Pheet> const& other) const;
};

template <class Pheet>
CentralizedPrioritySchedulerTaskStorageItem<Pheet>::CentralizedPrioritySchedulerTaskStorageItem()
: task(NULL), stack_element(NULL), priority(0) {

}

template <class Pheet>
bool CentralizedPrioritySchedulerTaskStorageItem<Pheet>::operator==(CentralizedPrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task == task;
}

template <class Pheet>
bool CentralizedPrioritySchedulerTaskStorageItem<Pheet>::operator!=(CentralizedPrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task != task;
}


template <class Pheet>
class nullable_traits<CentralizedPrioritySchedulerTaskStorageItem<Pheet> > {
public:
	static CentralizedPrioritySchedulerTaskStorageItem<Pheet> const null_value;
};

template <class Pheet>
CentralizedPrioritySchedulerTaskStorageItem<Pheet> const nullable_traits<CentralizedPrioritySchedulerTaskStorageItem<Pheet> >::null_value;

template <class Pheet, class TSI>
class CentralizedPrioritySchedulerTaskStorageItemComparator {
public:
	bool operator()(TSI const& first, TSI const& second) {
		return first.priority < second.priority;
	}
};

/*
 * May only be used once
 */
template <class Pheet, template <class, typename, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold = 4>
class CentralizedPrioritySchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
	using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef CentralizedPrioritySchedulerTaskStorageItem<Pheet> TaskStorageItem;
	typedef CentralizedPrioritySchedulerTaskStorageItemComparator<Pheet, TaskStorageItem> TaskStorageItemComparator;
	typedef TaskStorageT<Pheet, TaskStorageItem, TaskStorageItemComparator> TaskStorage;
	typedef CentralizedPrioritySchedulerPlace<Pheet, FinishStack, CallThreshold> Place;
	typedef CentralizedPrioritySchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
	typedef DefaultStrategyT<Pheet> DefaultStrategy;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	typedef bool StealerDescriptor;

	template <class NP>
	using BT = CentralizedPrioritySchedulerImpl<NP, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>;

	template<uint8_t NewVal>
		using WithCallThreshold = CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, NewVal>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = Self;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = CentralizedPrioritySchedulerImpl<Pheet, NewTS, FinishStack, DefaultStrategyT, CallThreshold>;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;

	/*
	 * Uses complete machine
	 */
	CentralizedPrioritySchedulerImpl();
	CentralizedPrioritySchedulerImpl(typename Place::PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	CentralizedPrioritySchedulerImpl(procs_t num_places);
	CentralizedPrioritySchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters);
	~CentralizedPrioritySchedulerImpl();

	static void print_name();

//	static void print_performance_counter_values(Place::PerformanceCounters performance_counters);

//	static void print_performance_counter_headers();

	static Place* get_place();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void finish(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void call(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void spawn(F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_prio(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_prio(Strategy s, F&& f, TaskParams&& ... params);

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

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
char const CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::name[] = "CentralizedPriorityScheduler";

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
procs_t const CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::CentralizedPrioritySchedulerImpl()
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::CentralizedPrioritySchedulerImpl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::CentralizedPrioritySchedulerImpl(procs_t num_places)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::CentralizedPrioritySchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(task_storage, machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::~CentralizedPrioritySchedulerImpl() {
	delete places[0];
	delete[] places;
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::print_name() {
	std::cout << name << "<";
	Place::TaskStorage::print_name();
	std::cout << ", " << (int)CallThreshold << ">";
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
typename CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::Place* CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
procs_t CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
typename CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::Place* CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::spawn_prio(Strategy s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn_prio<CallTaskType>(s, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class Strategy, typename F, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::spawn_prio(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_prio(s, f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, typename> class TaskStorageT, template <class> class FinishStack, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void CentralizedPrioritySchedulerImpl<Pheet, TaskStorageT, FinishStack, DefaultStrategyT, CallThreshold>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

template<class Pheet>
using CentralizedPriorityScheduler = CentralizedPrioritySchedulerImpl<Pheet, GlobalLockHeap, MMFinishStack, LifoFifoStrategy, 3>;

}


#endif /* CENTRALIZEDPRIORITYSCHEDULER_H_ */
