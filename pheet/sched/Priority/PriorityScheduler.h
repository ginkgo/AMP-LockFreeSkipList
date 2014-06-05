/*
 * PriorityScheduler.h
 *
 *  Created on: 19.09.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PRIORITYSCHEDULER_H_
#define PRIORITYSCHEDULER_H_

#include "../../settings.h"
#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"
#include "../common/FinishRegion.h"
#include "PrioritySchedulerPlace.h"
#include "PrioritySchedulerStealerDescriptor.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"
#include "../../ds/PriorityTaskStorage/Modular/ModularTaskStorage.h"
#include "../strategies/LifoFifo/LifoFifoStrategy.h"

#include <stdint.h>
#include <limits>

namespace pheet {

template <class Pheet>
struct PrioritySchedulerState {
	PrioritySchedulerState();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
	typename Pheet::Scheduler::Task* startup_task;
};

template <class Pheet>
PrioritySchedulerState<Pheet>::PrioritySchedulerState()
: current_state(0), startup_task(NULL) {

}


template <class Pheet>
struct PrioritySchedulerTaskStorageItem {
	typedef typename Pheet::Environment::Place Place;

	PrioritySchedulerTaskStorageItem();

	typename Pheet::Environment::Place::Task* task;
	typename Pheet::Environment::Place::StackElement* stack_element;

	bool operator==(PrioritySchedulerTaskStorageItem<Pheet> const& other) const;
	bool operator!=(PrioritySchedulerTaskStorageItem<Pheet> const& other) const;
};

template <class Pheet>
PrioritySchedulerTaskStorageItem<Pheet>::PrioritySchedulerTaskStorageItem()
: task(NULL), stack_element(NULL) {

}

template <class Pheet>
bool PrioritySchedulerTaskStorageItem<Pheet>::operator==(PrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task == task;
}

template <class Pheet>
bool PrioritySchedulerTaskStorageItem<Pheet>::operator!=(PrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task != task;
}


template <class Pheet>
class nullable_traits<PrioritySchedulerTaskStorageItem<Pheet> > {
public:
	static PrioritySchedulerTaskStorageItem<Pheet> const null_value;
};

template <class Pheet>
PrioritySchedulerTaskStorageItem<Pheet> const nullable_traits<PrioritySchedulerTaskStorageItem<Pheet> >::null_value;

/*
 * May only be used once
 */
template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold = 4>
class PrioritySchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
	using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef PrioritySchedulerTaskStorageItem<Pheet> TaskStorageItem;
	typedef TaskStorageT<Pheet, TaskStorageItem> TaskStorage;
	typedef PrioritySchedulerPlace<Pheet, CallThreshold> Place;
	typedef PrioritySchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
	typedef PrioritySchedulerStealerDescriptor<Pheet> StealerDescriptor;
	typedef DefaultStrategyT<Pheet> DefaultStrategy;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	template <class NP>
	using BT = PrioritySchedulerImpl<NP, TaskStorageT, DefaultStrategyT, CallThreshold>;

	template<uint8_t NewVal>
		using WithCallThreshold = PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, NewVal>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = PrioritySchedulerImpl<Pheet, NewTS, DefaultStrategyT, CallThreshold>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;


	/*
	 * Uses complete machine
	 */
	PrioritySchedulerImpl();
	PrioritySchedulerImpl(typename Place::PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	PrioritySchedulerImpl(procs_t num_places);
	PrioritySchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters);
	~PrioritySchedulerImpl();

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

	State state;

	PerformanceCounters performance_counters;
};

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
char const PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::name[] = "PriorityScheduler";

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
procs_t const PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::PrioritySchedulerImpl()
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::PrioritySchedulerImpl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::PrioritySchedulerImpl(procs_t num_places)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::PrioritySchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::~PrioritySchedulerImpl() {
	delete places[0];
	delete[] places;
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::print_name() {
	std::cout << name << "<";
	Place::TaskStorage::print_name();
	std::cout << ", " << (int)CallThreshold << ">";
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
typename PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::Place* PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
procs_t PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
typename PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::Place* PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::spawn_prio(Strategy s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn_prio<CallTaskType>(s, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class Strategy, typename F, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::spawn_prio(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_prio(s, f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class P> class DefaultStrategyT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void PrioritySchedulerImpl<Pheet, TaskStorageT, DefaultStrategyT, CallThreshold>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

template<class Pheet>
using PriorityScheduler = PrioritySchedulerImpl<Pheet, ModularTaskStorage, LifoFifoStrategy, 3>;

}


#endif /* PRIORITYSCHEDULER_H_ */
