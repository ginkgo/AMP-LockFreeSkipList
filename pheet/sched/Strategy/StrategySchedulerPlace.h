/*
 * StrategyScheduler2Place.h
 *
 *  Created on: Mar 8, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYSCHEDULERPLACE_H_
#define STRATEGYSCHEDULERPLACE_H_

#include "StrategySchedulerPerformanceCounters.h"
#include "../common/PlaceBase.h"

namespace pheet {


template <class Place>
struct StrategySchedulerPlaceLevelDescription {
	Place** partners;
	procs_t num_partners;
	procs_t local_id;
	procs_t size;
	procs_t memory_level;
	procs_t global_id_offset;
};



template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
class StrategySchedulerPlace : public PlaceBase<Pheet> {
public:
	typedef StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold> Self;
	typedef Self Place;
	typedef StrategySchedulerPlaceLevelDescription<Self> LevelDescription;
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::Scheduler::Task Task;
	template <typename F>
		using FunctorTask = typename Pheet::Scheduler::template FunctorTask<F>;
	typedef FinishStackT<Pheet> FinishStack;
	typedef typename FinishStack::Element StackElement;
	typedef typename Pheet::Scheduler::TaskStorageItem TaskStorageItem;
	typedef typename Pheet::Scheduler::TaskStorage TaskStorage;
	typedef typename Pheet::Scheduler::Stealer Stealer;
//	typedef typename Pheet::Scheduler::TaskDesc TaskDesc;
//	typedef typename Pheet::Scheduler::PlaceDesc PlaceDesc;
	typedef StrategySchedulerPerformanceCounters<Pheet, typename TaskStorage::PerformanceCounters, typename Stealer::PerformanceCounters, typename FinishStack::PerformanceCounters> PerformanceCounters;
	typedef typename Pheet::Scheduler::InternalMachineModel InternalMachineModel;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;

	StrategySchedulerPlace(InternalMachineModel model, Place** places, procs_t num_places, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count);
	StrategySchedulerPlace(LevelDescription* levels, procs_t num_initialized_levels, InternalMachineModel model, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count);
	~StrategySchedulerPlace();

	void prepare_root();
	void join();

	procs_t get_id();

	static Self* get();

	template<class Strategy>
		bool spawn_to_call_check(Strategy&& s);

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
		void spawn_s(Strategy&& s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy&& s, F&& f, TaskParams&& ... params);

	procs_t get_distance(Self* other) const;
//	procs_t get_distance(Self* other, procs_t max_granularity_level);
//	procs_t get_max_distance() const;
//	procs_t get_max_distance(procs_t max_granularity_level);

	void start_finish_region();
	void end_finish_region();

	ptrdiff_t next_task_id() { return task_id++; }

	TaskStorage& get_task_storage() { return task_storage; }

	procs_t get_num_levels() { return num_levels; }

private:
	void initialize_levels();
	void grow_levels_structure();
	void run();
	void execute_task(Task* task, StackElement* parent);
	void main_loop();
	void process_queue();
	bool process_queue_until_finished(StackElement* parent);
	void wait_for_finish(StackElement* parent);

	InternalMachineModel machine_model;
	procs_t num_initialized_levels;
	procs_t num_levels;
	LevelDescription* levels;

	StackElement* current_task_parent;

	typename Pheet::Scheduler::State* scheduler_state;

	PerformanceCounters performance_counters;

//	PlaceDesc place_desc;
	TaskStorage task_storage;
	Stealer stealer;
	FinishStack finish_stack;

//	size_t spawn2call_counter;

	CPUThreadExecutor<Self> thread_executor;

	ptrdiff_t task_id;

	static THREAD_LOCAL Self* local_place;

//	friend class Pheet::Scheduler::Finish;

	template <class T>
		friend void execute_cpu_thread(T* param);
};

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
THREAD_LOCAL StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold> *
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::local_place = nullptr;

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::StrategySchedulerPlace(InternalMachineModel model, Place** places, procs_t num_places, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count)
: machine_model(model),
  num_initialized_levels(1), num_levels(find_last_bit_set(num_places)), levels(new LevelDescription[num_levels]),
  current_task_parent(nullptr),
  scheduler_state(scheduler_state),
  performance_counters(perf_count),
  task_storage(performance_counters.task_storage_performance_counters),
  stealer(task_storage, performance_counters.stealer_performance_counters),
  finish_stack(performance_counters.finish_stack_performance_counters),
//  spawn2call_counter(0),
  thread_executor(this),
  task_id(0) {

	// This is the root task execution context. It differs from the others in that it reuses the existing thread instead of creating a new one

	performance_counters.total_time.start_timer();

	pheet_assert(num_places <= model.get_num_leaves());
	levels[0].global_id_offset = 0;
	levels[0].memory_level = model.get_memory_level();
	levels[0].size = num_places;
	levels[0].partners = places;
	levels[0].num_partners = 0;
	// We have to initialize this now, as the value is already used by performance counters during initialization
	levels[0].local_id = 0;

	initialize_levels();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::StrategySchedulerPlace(LevelDescription* levels, procs_t num_initialized_levels, InternalMachineModel model, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count)
: machine_model(model),
  num_initialized_levels(num_initialized_levels), num_levels(num_initialized_levels + find_last_bit_set(levels[num_initialized_levels - 1].size >> 1)),
  levels(new LevelDescription[num_levels]),
  current_task_parent(nullptr),
  scheduler_state(scheduler_state),
  performance_counters(perf_count),
  task_storage(performance_counters.task_storage_performance_counters),
  stealer(task_storage, performance_counters.stealer_performance_counters),
  finish_stack(performance_counters.finish_stack_performance_counters),
//  spawn2call_counter(0),
  thread_executor(this) {

	memcpy(this->levels, levels, sizeof(LevelDescription) * num_initialized_levels);
	// We have to initialize this now, as the value is already used by performance counters during initialization
	this->levels[0].local_id = this->levels[num_initialized_levels - 1].global_id_offset;

	thread_executor.run();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::~StrategySchedulerPlace() {
	if(get_id() == 0) {
		end_finish_region();

		task_storage.make_empty();

		// we can shut down the scheduler
		scheduler_state->current_state = 2;

		performance_counters.task_time.stop_timer();
		performance_counters.total_time.stop_timer();

		scheduler_state->state_barrier.barrier(1, levels[0].size);

		for(procs_t i = num_levels - 1; i >= 1; --i) {
			levels[i].partners[0]->join();
			delete levels[i].partners[0];
		}

		machine_model.unbind();
		local_place = NULL;
	}
	delete[] levels;
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::prepare_root() {
	scheduler_state->state_barrier.signal(0);

	pheet_assert(local_place == NULL);
	local_place = this;

	scheduler_state->state_barrier.wait(0, levels[0].size);

	performance_counters.task_time.start_timer();
	start_finish_region();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::initialize_levels() {
	procs_t base_offset;
	procs_t size;

	pheet_assert(num_initialized_levels > 0);
	base_offset = levels[num_initialized_levels - 1].global_id_offset;
	size = levels[num_initialized_levels - 1].size;

	Place** places = levels[0].partners;

	while((size > 1) && (!machine_model.is_leaf())) {
		pheet_assert(machine_model.get_num_children() == 2);
		grow_levels_structure();

		InternalMachineModel child(machine_model.get_child(1));
		procs_t offset = child.get_node_offset();
		pheet_assert(offset > base_offset);
		if((offset - base_offset) < size) {
			levels[num_initialized_levels].size = size - (offset - base_offset);
			levels[num_initialized_levels].global_id_offset = offset;
			levels[num_initialized_levels].num_partners = offset - base_offset;
			levels[num_initialized_levels].partners = places + base_offset;
			levels[num_initialized_levels].memory_level = child.get_memory_level();

			places[offset] = new Place(levels, num_initialized_levels + 1, child, scheduler_state, performance_counters);

			machine_model = machine_model.get_child(0);
			levels[num_initialized_levels].size = offset - base_offset;
			levels[num_initialized_levels].global_id_offset = base_offset;
			levels[num_initialized_levels].num_partners = size - (offset - base_offset);
			levels[num_initialized_levels].partners = places + offset;
			levels[num_initialized_levels].memory_level = machine_model.get_memory_level();

			size = offset - base_offset;
			++num_initialized_levels;
		}
		else {
			machine_model = machine_model.get_child(0);
		}
	}

	pheet_assert(levels[num_initialized_levels - 1].size == 1);
	procs_t global_id = levels[num_initialized_levels - 1].global_id_offset;
	// Level 0 is already initialized in the constructor
	for(procs_t i = 1; i < num_initialized_levels; ++i) {
		levels[i].local_id = global_id - levels[i].global_id_offset;
	}
	num_levels = num_initialized_levels;
	machine_model.bind();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::grow_levels_structure() {
	if(num_initialized_levels == num_levels) {
		// We have allocated to little levels
		procs_t new_size = num_levels + find_last_bit_set(levels[num_levels - 1].size >> 1);
		pheet_assert(new_size > num_levels);
		LevelDescription* tmp = new LevelDescription[new_size];
		memcpy(tmp, levels, num_levels * sizeof(LevelDescription));
		delete[] levels;
		levels = tmp;
		num_levels = new_size;
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::join() {
	thread_executor.join();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>*
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get() {
	return local_place;
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
procs_t
StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get_id() {
	return levels[0].local_id;
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::run() {
	local_place = this;
	initialize_levels();

	// Releases all writes to this place to all other places. Will become visible after wait
	scheduler_state->state_barrier.signal(0);

	performance_counters.total_time.start_timer();

	scheduler_state->state_barrier.wait(0, levels[0].size);

	main_loop();

	scheduler_state->state_barrier.barrier(1, levels[0].size);
	local_place = NULL;

	// Join all partners that this thread created
	for(procs_t i = num_levels - 1; i >= 1; --i) {
		if(levels[i].global_id_offset != levels[i-1].global_id_offset) {
			break;
		}
		levels[i].partners[0]->join();
		delete levels[i].partners[0];
	}

	performance_counters.total_time.stop_timer();

	// Now we can safely finish execution
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::execute_task(Task* task, StackElement* parent) {
	parent = finish_stack.active_element(parent);

	// Store parent (needed for spawns inside the task)
	current_task_parent = parent;

	// Execute task
	performance_counters.task_time.start_timer();
	(*task)();
	performance_counters.task_time.stop_timer();

	// Check whether current_task_parent still is parent (if not, there is some error)
	pheet_assert(current_task_parent == parent);

	// Signal that we finished executing this task
	finish_stack.signal_completion(parent);
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::main_loop() {
	while(true) {
		// Make sure our queue is empty
		process_queue();

		{	// Local scope so we have a new backoff object
			Backoff bo;
			TaskStorageItem di;
			performance_counters.idle_time.start_timer();
			while(true) {
				// Finalize elements in stack
				// We do not steal from the last level as there are no partners
				procs_t level = num_levels - 1;
				while(level > 0) {
					// For all except the top level we assume num_partners > 0
					pheet_assert(levels[level].num_partners > 0);
					std::uniform_int_distribution<procs_t> n_r_gen(0, levels[level].num_partners - 1);
					procs_t next_rand = n_r_gen(this->get_rng());
					pheet_assert(next_rand < levels[level].num_partners);
					pheet_assert(levels[level].partners[next_rand] != this);

					performance_counters.num_steal_calls.incr();
					performance_counters.steal_time.start_timer();
				//	StealerDescriptor sd(levels[level].partners[next_rand], this, num_levels - 1);
					di = stealer.steal_from(levels[level].partners[next_rand]);
				//	di = levels[level].partners[next_rand]->task_storage.steal_push(this->task_storage, sd, performance_counters.task_storage_performance_counters);
				//	di = levels[level].partners[next_rand % levels[level].num_partners]->task_storage.steal();
					performance_counters.steal_time.stop_timer();

					if(di.task != NULL) {
						performance_counters.idle_time.stop_timer();
						execute_task(di.task, di.stack_element);
						delete di.task;
						break;
					}
					else {
						pheet_assert(task_storage.is_empty());
						performance_counters.num_unsuccessful_steal_calls.incr();
					}
					--level;
				}
				if(di.task == NULL) {
					pheet_assert(task_storage.is_empty());
					if(scheduler_state->current_state >= 2) {
						performance_counters.idle_time.stop_timer();
						return;
					}
//					task_storage.perform_maintenance(performance_counters.task_storage_performance_counters);
					bo.backoff();
				}
				else {
					break;
				}
			}
		}
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::wait_for_finish(StackElement* parent) {
	while(!finish_stack.unique(parent)) {
		// Make sure our queue is empty
		if(!process_queue_until_finished(parent))
		{	// Local scope so we have a new backoff object
			Backoff bo;
			TaskStorageItem di;
			while(true) {
				// Finalize elements in stack
				// We do not steal from the last level as there are no partners
				procs_t level = num_levels - 1;
				while(level > 0) {
					// For all except the top level we assume num_partners > 0
					pheet_assert(levels[level].num_partners > 0);
					std::uniform_int_distribution<procs_t> n_r_gen(0, levels[level].num_partners - 1);
					procs_t next_rand = n_r_gen(this->get_rng());
					pheet_assert(levels[level].partners[next_rand] != this);
					performance_counters.num_steal_calls.incr();
				//	StealerDescriptor sd(levels[level].partners[next_rand], this, num_levels - 1);
					di = stealer.steal_from(levels[level].partners[next_rand]);
				//	di = levels[level].partners[next_rand]->task_storage.steal_push(this->task_storage, sd, performance_counters.task_storage_performance_counters);
				//	di = levels[level].partners[next_rand % levels[level].num_partners]->task_storage.steal();

					if(di.task != NULL) {
						execute_task(di.task, di.stack_element);
						delete di.task;
						break;
					}
					else {
						pheet_assert(task_storage.is_empty());
						performance_counters.num_unsuccessful_steal_calls.incr();
					}
					--level;
				}
				if(di.task == NULL) {
					pheet_assert(task_storage.is_empty());
					if(finish_stack.unique(parent)) {
						return;
					}
//					task_storage.perform_maintenance(performance_counters.task_storage_performance_counters);
					bo.backoff();
				}
				else {
					break;
				}
			}
		}
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::process_queue() {
	TaskStorageItem di = task_storage.pop();
	while(di.task != NULL) {
		// Warning, no distinction between locally spawned tasks and remote tasks
		// But this makes it easier with the finish construct, etc.
		// Otherwise we would have to empty our deque on the next finish call
		// which is bad for balancing
		execute_task(di.task, di.stack_element);
		delete di.task;
		di = task_storage.pop();
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
bool StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::process_queue_until_finished(StackElement* parent) {
	TaskStorageItem di = task_storage.pop();
	while(di.task != NULL) {
		// Warning, no distinction between locally spawned tasks and remote tasks
		// But this makes it easier with the finish construct, etc.
		// Otherwise we would have to empty our deque on the next finish call
		// which is bad for balancing
		execute_task(di.task, di.stack_element);
		delete di.task;
		if(finish_stack.unique(parent)) {
			return true;
		}
		di = task_storage.pop();
	}
	return false;
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::start_finish_region() {
	performance_counters.task_time.stop_timer();
	performance_counters.num_finishes.incr();

	current_task_parent = finish_stack.create_blocking(current_task_parent);

	performance_counters.task_time.start_timer();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::end_finish_region() {
	performance_counters.task_time.stop_timer();

	// Make backup of parent since parent might change while waiting
	StackElement* parent = current_task_parent;

	// Process other tasks until this task has been finished
	wait_for_finish(parent);

	// Restore old parent
	current_task_parent = finish_stack.destroy_blocking(parent);

	performance_counters.task_time.start_timer();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::finish(TaskParams&& ... params) {
	start_finish_region();

	call<CallTaskType>(std::forward<TaskParams&&>(params) ...);

	end_finish_region();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::finish(F&& f, TaskParams&& ... params) {
	start_finish_region();

	call(f, std::forward<TaskParams&&>(params) ...);

	end_finish_region();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::spawn(TaskParams&& ... params) {
	spawn_s<CallTaskType>(BaseStrategy(), std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::spawn(F&& f, TaskParams&& ... params) {
	spawn_s(BaseStrategy(), f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class Strategy>
inline bool StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::spawn_to_call_check(Strategy&& s) {
	size_t weight = (s.get_transitive_weight());
	size_t current_tasks = task_storage.size();
	size_t threshold = (current_tasks * current_tasks);

	return weight <= threshold;
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::spawn_s(Strategy&& s, TaskParams&& ... params) {
	performance_counters.num_spawns.incr();

	if(task_storage.is_full()) {
		// Rigid limit in case the data-structure cannot grow
		performance_counters.num_spawns_to_call.incr();
		call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
	}
	else {
		// TUNE: offsets and parameters
		if(s.forbid_call_conversion() || !spawn_to_call_check(s)) {
			pheet_assert(s.get_transitive_weight() > 0);
		//	spawn2call_counter = 0;
			performance_counters.num_actual_spawns.incr();
			CallTaskType* task = new CallTaskType(params ...);
			pheet_assert(current_task_parent != NULL);
//			pheet_assert(current_task_parent >= stack && (current_task_parent < (stack + stack_size)));
			finish_stack.spawn(current_task_parent);
			TaskStorageItem di;
			di.task = task;
			di.stack_element = current_task_parent;
			task_storage.push(std::forward<Strategy&&>(s), di);
		}
		else {
			performance_counters.num_spawns_to_call.incr();
			call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
		}
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class Strategy, typename F, typename ... TaskParams>
inline void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::spawn_s(Strategy&& s, F&& f, TaskParams&& ... params) {
	performance_counters.num_spawns.incr();

	if(task_storage.is_full()) {
		// Rigid limit in case the data-structure cannot grow
		performance_counters.num_spawns_to_call.incr();
		call(f, std::forward<TaskParams&&>(params) ...);
	}
	else {
		// TUNE: offsets and parameters
		if(s.forbid_call_conversion() || !spawn_to_call_check(s)) {
		//	spawn2call_counter = 0;
			pheet_assert(s.get_transitive_weight() > 0);

			performance_counters.num_actual_spawns.incr();
			auto bound = std::bind(f, params ...);

			FunctorTask<decltype(bound)>* task = new FunctorTask<decltype(bound)>(bound);
			pheet_assert(current_task_parent != NULL);
//			pheet_assert(current_task_parent >= stack && (current_task_parent < (stack + stack_size)));
			finish_stack.spawn(current_task_parent);
			TaskStorageItem di;
			di.task = task;
			di.stack_element = current_task_parent;
			task_storage.push(std::forward<Strategy&&>(s), di);
		}
		else {
			performance_counters.num_spawns_to_call.incr();
			call(f, std::forward<TaskParams&&>(params) ...);
		}
	}
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<class CallTaskType, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::call(TaskParams&& ... params) {
	performance_counters.num_calls.incr();
	// Create task
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	// Execute task
	task();
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
template<typename F, typename ... TaskParams>
void StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::call(F&& f, TaskParams&& ... params) {
	performance_counters.num_calls.incr();
	// Execute task
	f(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
procs_t StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get_distance(Self* other) const {
	if(other == this) {
		return 0;
	}

	procs_t offset = std::max(levels[num_levels - 1].memory_level, other->levels[other->num_levels - 1].memory_level);
	procs_t i = std::min(num_levels - 1, other->num_levels - 1);
	while(levels[i].global_id_offset != other->levels[i].global_id_offset) {
		pheet_assert(i > 0);
		--i;
	}
	pheet_assert(levels[i].memory_level <= offset);
	return offset - levels[i].memory_level;
}

/*
template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
procs_t StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get_distance(Self* other, procs_t max_granularity_level) {
	pheet_assert(max_granularity_level < num_levels);

	procs_t offset = std::max(levels[max_granularity_level].memory_level, other->levels[max_granularity_level].memory_level);
	procs_t i = max_granularity_level;
	while(levels[i].global_id_offset != other->levels[i].global_id_offset) {
		pheet_assert(i > 0);
		--i;
	}
	pheet_assert(levels[i].memory_level <= offset);
	return offset - levels[i].memory_level;
}
*/
/*
template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
inline procs_t StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get_max_distance() {
	return this->levels[num_levels - 1].memory_level - this->levels[0].memory_level;
}
*/
/*
template <class Pheet, template <class> class FinishStackT, uint8_t CallThreshold>
inline procs_t StrategySchedulerPlace<Pheet, FinishStackT, CallThreshold>::get_max_distance(procs_t max_granularity_level) {
	pheet_assert(max_granularity_level < num_levels);

	return this->levels[max_granularity_level].memory_level;
}*/

}

#endif /* STRATEGYSCHEDULERPLACE_H_ */
