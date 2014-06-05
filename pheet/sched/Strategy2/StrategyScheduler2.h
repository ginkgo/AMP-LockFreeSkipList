/*
 * StrategyScheduler2.h
 *
 *  Created on: Aug 12, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYSCHEDULER2_H_
#define STRATEGYSCHEDULER2_H_

#include "StrategyScheduler2Place.h"
#include "StrategyScheduler2PerformanceCounters.h"
#include "StrategyScheduler2TaskStorageItem.h"
#include "StrategyScheduler2BaseStrategy.h"

#include "../../settings.h"
#include "../../models/MachineModel/BinaryTree/BinaryTreeMachineModel.h"
#include "../common/SchedulerTask.h"
#include "../common/SchedulerFunctorTask.h"

#include <pheet/ds/FinishStack/MM/MMFinishStack.h>
#include <pheet/ds/StrategyTaskStorage/Strategy2Base/Strategy2BaseTaskStorage.h>

namespace pheet {

template <class Pheet>
struct StrategyScheduler2State {
	StrategyScheduler2State();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
	typename Pheet::Scheduler::Task* startup_task;
};

template <class Pheet>
StrategyScheduler2State<Pheet>::StrategyScheduler2State()
: current_state(0), startup_task(NULL) {

}


template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
class StrategyScheduler2Impl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
	using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef StrategyScheduler2TaskStorageItem<Pheet, Task, typename FinishStack<Pheet>::Element> TaskStorageItem;
	typedef TaskStorageT<Pheet, TaskStorageItem> TaskStorage;
	typedef typename TaskStorage::BaseTaskStorage BaseTaskStorage;
	typedef StrategyScheduler2Place<Pheet, FinishStack, 4> Place;
	typedef StrategyScheduler2State<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
//	typedef StrategyScheduler2PlaceDescriptor<Pheet> PlaceDesc;
//	template <class Strategy>
//	using TaskDesc = StrategyScheduler2TaskDescriptor<Pheet, Strategy>;
	typedef StrategyScheduler2BaseStrategy<Pheet> BaseStrategy;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	template <class NP>
	using BT = StrategyScheduler2Impl<NP, TaskStorageT, FinishStack>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = StrategyScheduler2Impl<Pheet, NewTS, FinishStack>;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = Self;

	/*
	 * Uses complete machine
	 */
	StrategyScheduler2Impl();
	StrategyScheduler2Impl(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	StrategyScheduler2Impl(procs_t num_places);
	StrategyScheduler2Impl(procs_t num_places, PerformanceCounters& performance_counters);
	~StrategyScheduler2Impl();

	static void print_name();

//	static void print_performance_counter_values(Place::PerformanceCounters performance_counters);

//	static void print_performance_counter_headers();

	static Self* get() {
		return singleton;
	}
	static Place* get_place();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);
	procs_t get_num_places() { return num_places; }

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
		void spawn_s(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy s, F&& f, TaskParams&& ... params);
/*
	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_s(Strategy&& s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy&& s, F&& f, TaskParams&& ... params);
*/
	static char const name[];
	static procs_t const max_cpus;

private:
	InternalMachineModel machine_model;

	Place** places;
	procs_t num_places;
	TaskStorage task_storage;

	State state;

	PerformanceCounters performance_counters;
	static Self* singleton;
};


template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
char const StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::name[] = "StrategyScheduler2";

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
procs_t const StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>* StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::singleton = nullptr;

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::StrategyScheduler2Impl()
: num_places(machine_model.get_num_leaves()), task_storage() {
	pheet_assert(singleton == nullptr);
	singleton = this;

	places = new Place*[num_places];
	places[0] = new Place(machine_model, &task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::StrategyScheduler2Impl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()), task_storage() {
	pheet_assert(singleton == nullptr);
	singleton = this;

	places = new Place*[num_places];
	places[0] = new Place(machine_model, &task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::StrategyScheduler2Impl(procs_t num_places)
: num_places(num_places), task_storage() {
	pheet_assert(singleton == nullptr);
	singleton = this;

	places = new Place*[num_places];
	places[0] = new Place(machine_model, &task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::StrategyScheduler2Impl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places), task_storage() {
	pheet_assert(singleton == nullptr);
	singleton = this;

	places = new Place*[num_places];
	places[0] = new Place(machine_model, &task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::~StrategyScheduler2Impl() {
	delete places[0];
	delete[] places;

	singleton = nullptr;
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::print_name() {
	std::cout << name << "<";
	TaskStorage::print_name();
	std::cout /*<< ", " << (int)CallThreshold*/ << ">";
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
typename StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::Place* StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
procs_t StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
typename StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::Place* StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class CallTaskType, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<typename F, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class CallTaskType, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<typename F, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn_s(Strategy s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn_s<CallTaskType>(std::move(s), std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class Strategy, typename F, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn_s(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::move(s), f, std::forward<TaskParams&&>(params) ...);
}
/*
template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn_s(Strategy&& s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s<CallTaskType>(std::forward<Strategy&&>(s), std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class Strategy, typename F, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::spawn_s(Strategy&& s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::forward<Strategy&&>(s), f, std::forward<TaskParams&&>(params) ...);
}*/

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<class CallTaskType, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T> class TaskStorageT, template <class> class FinishStack>
template<typename F, typename ... TaskParams>
inline void StrategyScheduler2Impl<Pheet, TaskStorageT, FinishStack>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}


template<class Pheet>
using StrategyScheduler2 = StrategyScheduler2Impl<Pheet, Strategy2BaseTaskStorage, MMFinishStack>;

}

#endif /* STRATEGYSCHEDULER2_H_ */
