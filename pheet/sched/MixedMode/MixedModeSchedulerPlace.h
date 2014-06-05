/*
 * MixedModeSchedulerPlace.h
 *
 *  Created on: 16.06.2011
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_
#define BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_

#include "../../settings.h"
#include "../common/CPUThreadExecutor.h"
#include "../common/FinishRegion.h"
#include "../common/PlaceBase.h"
#include "../../misc/atomics.h"
#include "../../misc/bitops.h"
#include "../../misc/type_traits.h"
#include "MixedModeSchedulerPerformanceCounters.h"

#include <vector>
#include <queue>
#include <iostream>

#include <random>
#include <functional>

/*
 *
 */
namespace pheet {

union MixedModeSchedulerPlaceRegistration {
	uint64_t complete;
	struct {
	//	uint32_t c;	// counter
		uint32_t r;	// required threads
		uint32_t a;	// acquired threads
	} parts;
};

struct MixedModeSchedulerPlaceFinishStackElement {
	// Modified by local thread. Incremented when task is spawned, decremented when finished
	size_t num_spawned;

	// Only modified by other threads. Stolen tasks that were finished (including spawned tasks)
	size_t num_finished_remote;

	// Pointer to num_finished_remote of another thread (the one we stole tasks from)
	MixedModeSchedulerPlaceFinishStackElement* parent;

	MixedModeSchedulerPlaceFinishStackElement* prev_el;

	// Counter to update atomically when finalizing this element (ABA problem)
	size_t version;
};

/*
 * Information about the next task to execute
 */
//
template <class Task, class StackElement>
struct MixedModeSchedulerPlaceTeamTaskData {
	// Task to execute
	Task* task;

	// Pointer to the next task to execute. set by coordinator
	MixedModeSchedulerPlaceTeamTaskData<Task, StackElement>* next;

	// Parent finish_stack element needed for signaling finish
	StackElement* parent;

	// Threads count down as soon as they finish executing the task.
	// The last thread to count down is responsible for cleanup of the task (signal finish, delete task)
	procs_t executed_countdown;

	// Size of the team to execute the task
	procs_t team_size;

	// Level of the team to execute the task
	procs_t team_level;
};

template <class Place>
struct MixedModeSchedulerPlaceTeamAnnouncement {
	typename Place::TeamTaskData* first_task;
	typename Place::TeamTaskData* last_task;

	Place* coordinator;
	MixedModeSchedulerPlaceTeamAnnouncement<Place>* next_team;

	typename Place::Registration reg;
	procs_t level;
};


struct MixedModeSchedulerPlaceTeamInfo {
	procs_t team_size;
	procs_t local_id;
	procs_t coordinator_id;
	procs_t team_level;
};

template <class Place>
struct MixedModeSchedulerPlaceLevelDescription {
	Place** partners;
	procs_t num_partners;
	procs_t local_id;
	procs_t size;
	procs_t memory_level;
	procs_t global_id_offset;
	procs_t spawn_same_size_threshold;
	bool reverse_ids;
};

template <class Place>
struct MixedModeSchedulerPlaceDequeItem {
	MixedModeSchedulerPlaceDequeItem();

	procs_t team_size;
	typename Place::Task* task;
	typename Place::FinishStackElement* finish_stack_element;

	bool operator==(MixedModeSchedulerPlaceDequeItem<Place> const& other) const;
	bool operator!=(MixedModeSchedulerPlaceDequeItem<Place> const& other) const;
};

template <class Place>
MixedModeSchedulerPlaceDequeItem<Place>::MixedModeSchedulerPlaceDequeItem()
: task(NULL), finish_stack_element(NULL) {

}

template <class Place>
bool MixedModeSchedulerPlaceDequeItem<Place>::operator==(MixedModeSchedulerPlaceDequeItem<Place> const& other) const {
	return other.task == task;
}

template <class Place>
bool MixedModeSchedulerPlaceDequeItem<Place>::operator!=(MixedModeSchedulerPlaceDequeItem<Place> const& other) const {
	return other.task != task;
}


template <class Place>
class nullable_traits<MixedModeSchedulerPlaceDequeItem<Place> > {
public:
	static MixedModeSchedulerPlaceDequeItem<Place> const null_value;
};

template <class Place>
MixedModeSchedulerPlaceDequeItem<Place> const nullable_traits<MixedModeSchedulerPlaceDequeItem<Place> >::null_value;

template <class Pheet, template <class SP, typename T> class StealingDequeT>
class MixedModeSchedulerPlace : public PlaceBase<Pheet> {
public:
	typedef MixedModeSchedulerPlace<Pheet, StealingDequeT> Self;
	typedef MixedModeSchedulerPlaceRegistration Registration;
	typedef Self Place;
	typedef MixedModeSchedulerPlaceLevelDescription<Self> LevelDescription;
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::Scheduler::Task Task;
	typedef MixedModeSchedulerPlaceFinishStackElement FinishStackElement;
	typedef MixedModeSchedulerPlaceDequeItem<Self> DequeItem;
	typedef StealingDequeT<Pheet, DequeItem> StealingDeque;
	typedef MixedModeSchedulerPlaceTeamTaskData<Task, FinishStackElement> TeamTaskData;
	typedef MixedModeSchedulerPlaceTeamInfo TeamInfo;
	typedef MixedModeSchedulerPlaceTeamAnnouncement<Place> TeamAnnouncement;
	typedef MixedModeSchedulerPerformanceCounters<Pheet, typename StealingDeque::PerformanceCounters> PerformanceCounters;
	typedef typename Pheet::Scheduler::InternalMachineModel InternalMachineModel;

	MixedModeSchedulerPlace(InternalMachineModel model, Place** places, procs_t num_places, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count);
	MixedModeSchedulerPlace(LevelDescription* levels, procs_t num_initialized_levels, InternalMachineModel model, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count);
//	MixedModeSchedulerPlace(std::vector<LevelDescription*> const* levels, std::vector<typename CPUHierarchy::CPUDescriptor*> const* cpus, typename Scheduler::State* scheduler_state, MixedModeSchedulerPerformanceCounters<Pheet>& perf_count);
	~MixedModeSchedulerPlace();

	void prepare_root();
	void join();

	procs_t get_id();

	static Place* get();

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void local_spawn(TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void finish_nt(procs_t nt_size, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call_nt(procs_t nt_size, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn_nt(procs_t nt_size, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void local_spawn_nt(procs_t nt_size, TaskParams&& ... params);

	// If team is out of sync
	void sync_team();
//	void team_barrier();

	bool is_coordinator();
	procs_t get_global_id();
	procs_t get_local_id();
	procs_t get_coordinator_id();
	procs_t get_team_size();
	procs_t get_max_team_size();

	void start_finish_region();
	void end_finish_region();
private:
	void run();
	void init();
	void initialize_levels();
	void grow_levels_structure();

	void wait_for_shutdown();
	void wait_for_finish(FinishStackElement* parent);
	void wait_for_sync();
	void wait_for_coordinator_finish(TeamTaskData const* parent_task);
	void coordinate_team();
	bool coordinate_team_until_finished(FinishStackElement* parent);
	void coordinate_team_level();
	bool coordinate_team_level_until_finished(FinishStackElement* parent);
	void disband_team();
	bool execute_next_queue_task();
	bool execute_next_queue_task(procs_t min_level);
	void execute_queue_task(DequeItem const& di);
	void execute_solo_queue_task(DequeItem const& di);
	void execute_team_task(TeamTaskData* team_task);
	void create_team(procs_t team_size);
	TeamTaskData* create_team_task(DequeItem di);
	TeamTaskData* create_solo_team_task(DequeItem di);
	void announce_first_team_task(TeamTaskData* team_task);
	void announce_next_team_task(TeamTaskData* team_task);
	FinishStackElement* create_non_blocking_finish_region(FinishStackElement* parent);
	void empty_finish_stack();
	void signal_task_completion(FinishStackElement* finish_stack_element);
	void finalize_finish_stack_element(FinishStackElement* element, FinishStackElement* parent, size_t version, bool local);
	void visit_partners();
	void visit_partners_until_finished(FinishStackElement* parent);
	void visit_partners_until_synced(TeamAnnouncement* team);
	TeamAnnouncement* find_partner_team(Place* partner, procs_t level);
	void join_team(TeamAnnouncement* team);
	bool tie_break_team(TeamAnnouncement* my_team, TeamAnnouncement* other_team);
	void follow_team();
	void prepare_team_info(TeamAnnouncement* team);
	void prepare_solo_team_info();
	procs_t get_level_for_num_threads(procs_t num_threads);
	bool has_local_work();
	bool has_local_work(procs_t min_level);
	DequeItem get_next_team_task();
	DequeItem get_next_queue_task();
	DequeItem get_next_queue_task(procs_t min_level);
	DequeItem steal_tasks_from_partner(Place* partner, procs_t min_level);
	DequeItem steal_for_sync(TeamAnnouncement* my_team, Place* partner, procs_t min_level);
	void store_item_in_deque(DequeItem di, procs_t level);

	void register_for_team(TeamAnnouncement* team);
	bool deregister_from_team(TeamAnnouncement* team);

	InternalMachineModel machine_model;

	// machine hierarchy
	procs_t num_initialized_levels;
	procs_t num_levels;
	LevelDescription* levels;
//	procs_t* level_map;

	// Stack is only used by the coordinator
	static size_t const finish_stack_size;
	FinishStackElement* finish_stack;
	size_t finish_stack_filled_left;
	size_t finish_stack_filled_right;
	size_t finish_stack_init_left;

	// Team task information
	TeamTaskData* current_team_task;
	TeamAnnouncement* current_team;
//	TeamAnnouncement** announced_teams;
//	procs_t team_announcement_index;

	// Information needed for task reclamation scheme
	// TODO: we might get rid of the queue alltogether and use a single pointer instead as the queue never gets more than one entry (i think)
	std::queue<TeamTaskData*> team_task_reclamation_queue;
	TeamTaskData** team_task_reuse;

	// Information needed for team reclamation scheme
	std::queue<TeamAnnouncement*> team_announcement_reclamation_queue;
	TeamAnnouncement** team_announcement_reuse;

	// for finishing
	TeamTaskData const* waiting_for_finish;

	// Information about the team (calculated by all threads)
	TeamInfo* team_info;
	TeamInfo* default_team_info;
	TeamInfo* solo_team_info;

	StealingDequeT<Pheet, DequeItem>** stealing_deques;
	StealingDequeT<Pheet, DequeItem>** lowest_level_deque;
	StealingDequeT<Pheet, DequeItem>** highest_level_deque;
//	StealingDequeT<DequeItem>** current_deque;

	typename Pheet::Scheduler::State* scheduler_state;

	PerformanceCounters performance_counters;

	CPUThreadExecutor<Self> thread_executor;

	static THREAD_LOCAL Place* local_place;

	template <class T>
		friend void execute_cpu_thread(T* param);
//	friend class Pheet::Finish;
};

template <class Pheet, template <class SP, typename T> class StealingDequeT>
size_t const MixedModeSchedulerPlace<Pheet, StealingDequeT>::finish_stack_size = 8192;

template <class Pheet, template <class SP, typename T> class StealingDequeT>
THREAD_LOCAL MixedModeSchedulerPlace<Pheet, StealingDequeT> *
MixedModeSchedulerPlace<Pheet, StealingDequeT>::local_place = NULL;

template <class Pheet, template <class SP, typename T> class StealingDequeT>
MixedModeSchedulerPlace<Pheet, StealingDequeT>::MixedModeSchedulerPlace(InternalMachineModel model, Place** places, procs_t num_places, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count)
: machine_model(model),
  num_initialized_levels(1), num_levels(find_last_bit_set(num_places)), levels(new LevelDescription[num_levels]),
//  current_task_parent(nullptr),
  finish_stack_filled_left(0), finish_stack_filled_right(finish_stack_size), finish_stack_init_left(0),
  current_team_task(nullptr), current_team(nullptr),
  waiting_for_finish(nullptr), team_info(nullptr),
  stealing_deques(nullptr), lowest_level_deque(nullptr), highest_level_deque(NULL),
  scheduler_state(scheduler_state),
  performance_counters(perf_count),
  thread_executor(this) {

	// This is the root task execution context. It differs from the others in that it reuses the existing thread instead of creating a new one

	performance_counters.total_time.start_timer();

	pheet_assert(num_places <= model.get_num_leaves());
	levels[0].global_id_offset = 0;
	levels[0].memory_level = model.get_memory_level();
	levels[0].size = num_places;
	levels[0].partners = places;
	levels[0].num_partners = 0;
	levels[0].reverse_ids = false;
	// We have to initialize this now, as the value is already used by performance counters during initialization
	levels[0].local_id = 0;

	initialize_levels();
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
MixedModeSchedulerPlace<Pheet, StealingDequeT>::MixedModeSchedulerPlace(LevelDescription* levels, procs_t num_initialized_levels, InternalMachineModel model, typename Pheet::Scheduler::State* scheduler_state, PerformanceCounters& perf_count)
: machine_model(model),
  num_initialized_levels(num_initialized_levels), num_levels(num_initialized_levels + find_last_bit_set(levels[num_initialized_levels - 1].size >> 1)),
  levels(new LevelDescription[num_levels]),
//  current_task_parent(nullptr),
  finish_stack_filled_left(0), finish_stack_filled_right(finish_stack_size), finish_stack_init_left(0),
  current_team_task(nullptr), current_team(nullptr),
  waiting_for_finish(nullptr), team_info(nullptr),
  stealing_deques(nullptr), lowest_level_deque(nullptr), highest_level_deque(NULL),
  scheduler_state(scheduler_state),
  performance_counters(perf_count),
  thread_executor(this) {

	memcpy(this->levels, levels, sizeof(LevelDescription) * num_initialized_levels);
	// We have to initialize this now, as the value is already used by performance counters during initialization
	this->levels[0].local_id = this->levels[num_initialized_levels - 1].global_id_offset;

	thread_executor.run();
}
/*
template <class Pheet, template <class SP, typename T> class StealingDequeT>
MixedModeSchedulerPlace<Pheet, StealingDequeT>::MixedModeSchedulerPlace(std::vector<LevelDescription*> const* levels, std::vector<typename CPUHierarchy::CPUDescriptor*> const* cpus, typename Scheduler::State* scheduler_state, MixedModeSchedulerPerformanceCounters<Pheet>& perf_count)
: performance_counters(perf_count),
  finish_stack_filled_left(0), finish_stack_filled_right(finish_stack_size), finish_stack_init_left(0), num_levels(levels->size()), current_team_task(NULL), current_team(NULL),
  waiting_for_finish(NULL), team_info(NULL), lowest_level_deque(NULL),
  highest_level_deque(NULL), thread_executor(cpus, this), scheduler_state(scheduler_state)
  {
	performance_counters.total_time.start_timer();

	this->levels = new LevelDescription[num_levels];
	procs_t local_id = 0;
	for(ptrdiff_t i = num_levels - 1; i >= 0; i--) {
		this->levels[i].partners = (*levels)[i]->partners;
		this->levels[i].num_partners = (*levels)[i]->num_partners;
		local_id += (*levels)[i]->local_id;
		this->levels[i].local_id = local_id;
		this->levels[i].size = (*levels)[i]->size;
		this->levels[i].reverse_ids = (*levels)[i]->reverse_ids;
	}

	// May be moved to the parallel part if we add a full barrier after initialization. Not sure if it is worth it though
	stealing_deques = new StealingDequeT<Pheet, DequeItem>*[num_levels];
	for(procs_t i = 0; i < num_levels; ++i) {
		procs_t size = find_last_bit_set(this->levels[num_levels - i - 1].size) << 4;
		stealing_deques[i] = new StealingDequeT<Pheet, DequeItem>(size, performance_counters.num_steals, performance_counters.num_stealing_deque_pop_cas);
		this->levels[i].spawn_same_size_threshold = size;
	}

	// All other initializations can be done in parallel

	// Run thread
	thread_executor.run();
}*/

template <class Pheet, template <class SP, typename T> class StealingDequeT>
MixedModeSchedulerPlace<Pheet, StealingDequeT>::~MixedModeSchedulerPlace() {
	if(get_id() == 0) {
		end_finish_region();
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
		local_place = nullptr;
	}
	delete[] finish_stack;
	delete[] levels;
//	delete[] level_map;
//	delete[] announced_teams;

	delete default_team_info;
	delete solo_team_info;

	while(!team_task_reclamation_queue.empty()) {
		delete team_task_reclamation_queue.front();
		team_task_reclamation_queue.pop();
	}

	while(!team_announcement_reclamation_queue.empty()) {
		delete team_announcement_reclamation_queue.front();
		team_announcement_reclamation_queue.pop();
	}

	for(procs_t i = 0; i < num_levels; ++i) {
		delete team_task_reuse[i];
		delete team_announcement_reuse[i];
		delete stealing_deques[i];
	}
	delete[] team_task_reuse;
	delete[] team_announcement_reuse;
	delete[] stealing_deques;
}

template <class Pheet, template <class P, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::prepare_root() {
	scheduler_state->state_barrier.signal(0);

	pheet_assert(local_place == nullptr);
	local_place = this;

	init();

	create_team(1);
	start_finish_region();

	DequeItem di;
	di.finish_stack_element = &(finish_stack[finish_stack_filled_right]);
	di.task = nullptr;
	di.team_size = 1;
	current_team_task = create_solo_team_task(di);

	scheduler_state->state_barrier.wait(0, levels[0].size);

	performance_counters.task_time.start_timer();
}

template <class Pheet, template <class P, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::initialize_levels() {
	procs_t base_offset;
	procs_t size;

	pheet_assert(num_initialized_levels > 0);
	base_offset = levels[num_initialized_levels - 1].global_id_offset;
	size = levels[num_initialized_levels - 1].size;

	if(num_initialized_levels > 0) {
		levels[num_initialized_levels - 1].reverse_ids = true;
	}
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
			levels[num_initialized_levels].reverse_ids = false;

			places[offset] = new Place(levels, num_initialized_levels + 1, child, scheduler_state, performance_counters);

			machine_model = machine_model.get_child(0);
			levels[num_initialized_levels].size = offset - base_offset;
			levels[num_initialized_levels].global_id_offset = base_offset;
			levels[num_initialized_levels].num_partners = size - (offset - base_offset);
			levels[num_initialized_levels].partners = places + offset;
			levels[num_initialized_levels].memory_level = machine_model.get_memory_level();
			levels[num_initialized_levels].reverse_ids = false;

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

template <class Pheet, template <class P, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::grow_levels_structure() {
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

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::join() {
	thread_executor.join();
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
MixedModeSchedulerPlace<Pheet, StealingDequeT>*
MixedModeSchedulerPlace<Pheet, StealingDequeT>::get() {
	return local_place;
}

template <class Pheet, template <class P, typename T> class StealingDequeT>
procs_t
MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_id() {
	return levels[0].local_id;
}

/*
 * Initialization of stuff that can be done in parallel
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::init() {
	stealing_deques = new StealingDequeT<Pheet, DequeItem>*[num_levels];
	for(procs_t i = 0; i < num_levels; ++i) {
		procs_t size = find_last_bit_set(this->levels[num_levels - i - 1].size) << 4;
		stealing_deques[i] = new StealingDequeT<Pheet, DequeItem>(size, performance_counters.stealing_deque_performance_counters);
		this->levels[i].spawn_same_size_threshold = size;
	}

	finish_stack = new FinishStackElement[finish_stack_size];

	// Create map for simple lookup of levels in the hierarchy (if we have a number of threads)
/*	procs_t max_levels = find_last_bit_set(this->levels[0].size - 1) + 1;
	level_map = new procs_t[max_levels];
	procs_t lvl = num_levels - 1;
	level_map[0] = num_levels - 1;
	for(procs_t i = 1; i < max_levels - 1; i++) {
		procs_t min_size = (1 << (i - 1)) + 1;
		procs_t lvl_size = this->levels[lvl].size;
		while(min_size > lvl_size) {
			--lvl;
			lvl_size = this->levels[lvl].size;
		}
		level_map[i] = lvl;
	}
	level_map[max_levels - 1] = 0;*/

	// Initialize stuff for memory reuse
//	announced_teams = new TeamAnnouncement*[num_levels];
	team_task_reuse = new TeamTaskData*[num_levels];
	team_announcement_reuse = new TeamAnnouncement*[num_levels];
	for(procs_t i = 0; i < num_levels; ++i) {
//		announced_teams[i] = NULL;

		team_task_reuse[i] = new TeamTaskData();
		team_announcement_reuse[i] = new TeamAnnouncement();
	}
	team_task_reuse[num_levels - 1]->team_level = num_levels - 1;
	team_task_reuse[num_levels - 1]->team_size = 1;
	team_task_reuse[num_levels - 1]->next = NULL;
	team_task_reuse[num_levels - 1]->executed_countdown = 0;
	team_announcement_reuse[num_levels - 1]->level = num_levels - 1;
	team_announcement_reuse[num_levels - 1]->coordinator = this;

	// Initialize team_info structure
	default_team_info = new TeamInfo();

	solo_team_info = new TeamInfo();
	solo_team_info->coordinator_id = 0;
	solo_team_info->local_id = 0;
	solo_team_info->team_level = num_levels - 1;
	solo_team_info->team_size = 1;

	team_info = solo_team_info;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::run() {
	local_place = this;
	initialize_levels();

	scheduler_state->state_barrier.signal(0);

	init();

	// Wait until all threads are initialized
	scheduler_state->state_barrier.wait(0, levels[0].size);

	// Start execution
/*	Task* startup_task = scheduler_state->startup_task;
	if(startup_task != NULL && PTR_CAS(&(scheduler_state->startup_task), startup_task, NULL)) {
		performance_counters.queue_processing_time.start_timer();
		start_finish_region();

		++finish_stack[finish_stack_filled_right].num_spawned;
		DequeItem di;
		di.finish_stack_element = &(finish_stack[finish_stack_filled_right]);
		di.task = startup_task;
		di.team_size = scheduler_state->team_size;

		create_team(di.team_size);

		performance_counters.queue_processing_time.stop_timer();
		if(di.team_size == 1) {
			execute_solo_queue_task(di);
		}
		else {
			execute_queue_task(di);
		}
		performance_counters.queue_processing_time.start_timer();

		if(di.finish_stack_element->num_spawned > 0) {
			// There exist some non-executed or stolen tasks

			// Execute the rest of the tasks in the team
			coordinate_team();

			performance_counters.queue_processing_time.stop_timer();

			// Process other tasks until this task has been finished
			wait_for_finish(di.finish_stack_element);
		}
		else {
			performance_counters.queue_processing_time.stop_timer();
		}
		if(current_team != NULL && current_team->level != num_levels - 1) {
			disband_team();
		}

		scheduler_state->current_state = 2; // finished
	}
	else {*/
		wait_for_shutdown();
//	}

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

	// We need to stop here, as it isn't safe after the barrier
	performance_counters.total_time.stop_timer();

	// Now we can safely finish execution
}

/*
 * Do work until the scheduler shuts down
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::wait_for_shutdown() {
	// pre-condition: queue must be empty
	pheet_assert(!has_local_work());

	while(scheduler_state->current_state != 2) {
		// Try to join teams or steal tasks
		visit_partners();

		performance_counters.queue_processing_time.start_timer();
		// Coordinate the current team if existing until it's empty
		coordinate_team();

		while(has_local_work()) {
			// Execute a single task. This will create a team if there was a task
			if(execute_next_queue_task()) {
				// Coordinate the current team if existing until it's empty
				coordinate_team();
			}
		}
		performance_counters.queue_processing_time.stop_timer();
	}
}

/*
 * Do work until the task has been finished
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::wait_for_finish(FinishStackElement* parent) {
	while(parent->num_finished_remote != parent->num_spawned) {
		// TODO: try a policy where we do not need to empty our queues before we notice the finish
		// (currently not implemented for simplicity)

		performance_counters.queue_processing_time.start_timer();
		while(has_local_work()) {
			// Execute a single task. This will create a team if there was a task
			if(execute_next_queue_task()) {
				// Coordinate the current team if existing until it's empty
				if(coordinate_team_until_finished(parent)) {
					performance_counters.queue_processing_time.stop_timer();
					return;
				}
			}
		}
		performance_counters.queue_processing_time.stop_timer();

		// Try to join teams or steal tasks
		visit_partners_until_finished(parent);

		performance_counters.queue_processing_time.start_timer();
		// Coordinate the current team if existing until it's empty
		if(coordinate_team_until_finished(parent)) {
			performance_counters.queue_processing_time.stop_timer();
			return;
		}
		performance_counters.queue_processing_time.stop_timer();
	}
}

/*
 * Do work until the current team is synchronized
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::wait_for_sync() {
	// TODO: make this work if a finish is invoked inside sync with thread requirement >= this

	// TODO: Let threads find the big team even though it is shadowed by a temporary team - (presumably higher performance)

	procs_t min_level = current_team->level + 1;
	TeamAnnouncement* my_team = current_team;
	TeamTaskData* my_team_task = current_team_task;
	TeamInfo* my_team_info = default_team_info;
	TeamInfo sub_team_info;
	default_team_info = &sub_team_info;
	do {
		if(has_local_work(min_level)) {
			if(!deregister_from_team(my_team)) {
				break;
			}

			performance_counters.queue_processing_time.start_timer();
			do {
				performance_counters.sync_time.stop_timer();
				// Execute a single task. This will create a team if there was a task
				if(execute_next_queue_task(min_level)) {
					// Coordinate the current team if existing until it's empty
//					performance_counters.queue_processing_time.stop_timer();
					coordinate_team();
	//				performance_counters.queue_processing_time.start_timer();
				}
				performance_counters.sync_time.start_timer();
			} while(has_local_work(min_level));
			performance_counters.queue_processing_time.stop_timer();
			// If we executed something, make sure we drop the team

			if(current_team != my_team && current_team != NULL && current_team->level != num_levels - 1) {
				disband_team();
			}

			register_for_team(my_team);
		//	--team_announcement_index;

			if(my_team->reg.parts.a == my_team->reg.parts.r) {
				break;
			}
		}

		pheet_assert(!has_local_work(min_level));

		// steal tasks from partners or join partner teams
		visit_partners_until_synced(my_team);

		if(current_team != my_team) {
			performance_counters.queue_processing_time.start_timer();
			// Coordinate the current team if existing until it's empty
			coordinate_team();
			if(current_team != NULL && current_team->level != num_levels - 1) {
				disband_team();
			}
			performance_counters.queue_processing_time.stop_timer();

			register_for_team(my_team);
		}
	}while(my_team->reg.parts.a != my_team->reg.parts.r);
	default_team_info = my_team_info;
	team_info = my_team_info;
//	current_team = my_team;
	current_team_task = my_team_task;
}

/*
 * Do work until the task has been finished
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::wait_for_coordinator_finish(TeamTaskData const* parent_task) {
	TeamTaskData const* prev_waiting_for_finish = waiting_for_finish;
	waiting_for_finish = parent_task;

	follow_team();

	waiting_for_finish = prev_waiting_for_finish;
}

/*
 * Coordinates a team until we run out of work for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::coordinate_team() {
	if(current_team != NULL) {
		procs_t level = current_team->level;
		coordinate_team_level();

		// TODO: get smaller tasks and just resize the team instead of disbanding it

		if(level < num_levels - 1) {
			disband_team();
		}
		else {
			current_team = NULL;
		}
	}
}

/*
 * Coordinates a team until we run out of work for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::coordinate_team_until_finished(FinishStackElement* parent) {
	if(current_team != NULL) {
		procs_t level = current_team->level;
		if(coordinate_team_level_until_finished(parent)) {
			return true;
		}

		// TODO: get smaller tasks and just resize the team instead of disbanding it

		if(level < num_levels - 1) {
			disband_team();
		}
		else {
			current_team = NULL;
		}
	}
	return false;
}

/*
 * Coordinates a team until we run out of work for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::coordinate_team_level() {
	if(current_team->level == num_levels - 1) {
		// Solo team
		DequeItem di = get_next_team_task();
		while(di.task != NULL) {
			performance_counters.queue_processing_time.stop_timer();
			// Execute
			execute_solo_queue_task(di);
			performance_counters.queue_processing_time.start_timer();

			// Try to get a same-size task
			di = get_next_team_task();
		}
		current_team = NULL;
	}
	else {
		DequeItem di = get_next_team_task();
		while(di.task != NULL) {
			// This method is responsible for creating the team task data and finish stack elements, etc...
			TeamTaskData* tt = create_team_task(di);

			// Show it to the rest of the team
			announce_next_team_task(tt);

			performance_counters.queue_processing_time.stop_timer();
			// Execute (same for coor and client!)
			execute_team_task(tt);
			performance_counters.queue_processing_time.start_timer();

			// Send task to memory reclamation
			team_task_reclamation_queue.push(tt);

			// Try to get a same-size task
			di = get_next_team_task();
		}
	}
}

/*
 * Coordinates a team until we run out of work for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::coordinate_team_level_until_finished(FinishStackElement* parent) {
	if(current_team->level == num_levels - 1) {
		// Solo team
		DequeItem di = get_next_team_task();
		while(di.task != NULL) {
			performance_counters.queue_processing_time.stop_timer();
			// Execute
			execute_solo_queue_task(di);
			performance_counters.queue_processing_time.start_timer();

			if(parent->num_spawned == parent->num_finished_remote) {
				return true;
			}

			// Try to get a same-size task
			di = get_next_team_task();
		}
		current_team = NULL;
	}
	else {
		DequeItem di = get_next_team_task();
		while(di.task != NULL) {
			// This method is responsible for creating the team task data and finish stack elements, etc...
			TeamTaskData* tt = create_team_task(di);

			// Show it to the rest of the team
			announce_next_team_task(tt);

			performance_counters.queue_processing_time.stop_timer();
			// Execute (same for coor and client!)
			execute_team_task(tt);
			performance_counters.queue_processing_time.start_timer();

			// Send task to memory reclamation
			team_task_reclamation_queue.push(tt);

			if(parent->num_spawned == parent->num_finished_remote) {
				return true;
			}

			// Try to get a same-size task
			di = get_next_team_task();
		}
	}
	return false;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::disband_team() {
	pheet_assert(current_team != NULL);
	pheet_assert(current_team->level != num_levels - 1);
	pheet_assert(current_team->coordinator == this);
	pheet_assert(current_team->reg.parts.r == current_team->reg.parts.a);

	// Put the old team into memory reclamation (as soon as it will be retrieved from reclamation,
	// it is guaranteed that no other (relevant) threads still have a reference to this
	team_announcement_reclamation_queue.push(current_team);

	// Create a dummy team with a single thread - Other threads will see this team and exit
	create_team(1);

	// Finally, drop out of this team
	current_team = NULL;
}

/*
 * Finds a single task, creates a team for it and executes the task
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::execute_next_queue_task() {
	DequeItem di = get_next_queue_task();

	if(di.task != NULL) {
		performance_counters.queue_processing_time.stop_timer();
		create_team(di.team_size);

		if(di.team_size == 1) {
			execute_solo_queue_task(di);
		}
		else {
			execute_queue_task(di);
		}
		performance_counters.queue_processing_time.start_timer();
		return true;
	}
	return false;
}

/*
 * Finds a single task, creates a team for it and executes the task
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::execute_next_queue_task(procs_t min_level) {
	DequeItem di = get_next_queue_task(min_level);

	if(di.task != NULL) {
		performance_counters.queue_processing_time.stop_timer();
		create_team(di.team_size);

		if(di.team_size == 1) {
			execute_solo_queue_task(di);
		}
		else {
			execute_queue_task(di);
		}
		performance_counters.queue_processing_time.start_timer();
		return true;
	}
	return false;
}

/*
 * executes the given task (if it is a team task, it is announced for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::execute_queue_task(DequeItem const& di) {
/*	if(di.team_size == 1) {
		execute_solo_task(di);
	}
	else {*/
	pheet_assert(di.team_size > 1);

	// This method is responsible for creating the team task data and finish stack elements, etc...
	TeamTaskData* tt = create_team_task(di);

	// Show it to the rest of the team
	announce_first_team_task(tt);

	// Execute (same for coor and client!)
	execute_team_task(current_team_task);

	// Send task to memory reclamation
	team_task_reclamation_queue.push(tt);
//	}
}

/*
 * executes the given task (if it is a team task, it is announced for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::execute_solo_queue_task(DequeItem const& di) {
	pheet_assert(di.team_size == 1);
//	performance_counters.num_tasks_at_level.incr(num_levels - 1);

	current_team_task = create_solo_team_task(di);

	performance_counters.task_time.start_timer();
	// Execute task
	(*di.task)();
	performance_counters.task_time.stop_timer();

	// signal that we finished execution of the task
	signal_task_completion(current_team_task->parent);

	delete di.task;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::execute_team_task(TeamTaskData* team_task) {
	performance_counters.task_time.start_timer();
	// Execute task
	(*(team_task->task))();

	// Signal that we finished executing this task
	if(PROCST_FETCH_AND_SUB(&(team_task->executed_countdown), 1) == 1) {
		// Last task to finish - do some cleanup

		// signal that we finished execution of the task
		signal_task_completion(team_task->parent);

		delete team_task->task;
	}
	performance_counters.task_time.stop_timer();
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::create_team(procs_t team_size) {
	procs_t level = get_level_for_num_threads(team_size);
	if(level == num_levels - 1) {
		TeamAnnouncement* team = team_announcement_reuse[num_levels - 1];
		if(current_team != NULL) {
			// First announce the next team so other threads in the current team know that this team is finished (which means they will call sync_team)
			current_team->last_task = current_team_task;
			current_team->next_team = team;

			// We have to make sure the team is synced first before changing the current_team pointer
			// New team smaller: make sure all threads joined the team (last chance for getting those threads in)
			sync_team();
//			announced_teams[team_announcement_index] = NULL;
		}
		current_team = team;
	//	current_deque = stealing_deques + num_levels - 1;
		prepare_solo_team_info();
	}
	else {
		TeamAnnouncement* team;

		if(!team_announcement_reclamation_queue.empty()) {
			pheet_assert(team_announcement_reclamation_queue.front()->reg.parts.r == team_announcement_reclamation_queue.front()->reg.parts.a);
			pheet_assert(team_announcement_reclamation_queue.front()->level < num_levels - 1);

			TeamAnnouncement* tmp = team_announcement_reclamation_queue.front();
			team_announcement_reclamation_queue.pop();
			team = team_announcement_reuse[tmp->level];
			team_announcement_reuse[tmp->level] = tmp;
		}
		else {
			team = new TeamAnnouncement();
		}

		team->coordinator = this;
		team->first_task = NULL;
		team->last_task = NULL;
		team->level = level;
		team->next_team = NULL;
		team->reg.parts.r = levels[level].size;
		if(current_team == NULL) {
			team->reg.parts.a = 1;
		}
		else {


			if(current_team->level > team->level) {
				team->reg.parts.a = current_team->reg.parts.r;
			}
			else {
				// Synchronized from the start - perfect
				team->reg.parts.a = levels[level].size;
			}
		}

		// TODO: If we have to sync the team, we can drop the fence
		MEMORY_FENCE();

		if(current_team != NULL) {
		/*	if(current_team->level > team->level) {
				// We now need more threads so we need to reannounce.
				announced_teams[team_announcement_index] = team;
			}
			else {
				pheet_assert(current_team->level != team->level);
				// This team is smaller so no need for reannouncement
				announced_teams[team_announcement_index] = NULL;
			}*/
			// First announce the next team so other threads in the current team know that this team is finished (which means they will call sync_team)
			current_team->last_task = current_team_task;
			current_team->next_team = team;

			// We have to make sure the team is synced first before changing the current_team pointer
			// New team smaller: make sure all threads joined the team (last chance for getting those threads in)
			// New team larger: Make sure all threads saw the announcement
			sync_team();
		}
	/*	else {
			announced_teams[team_announcement_index] = team;
		}*/
		current_team = team;
	//	current_deque = stealing_deques + level;

		prepare_team_info(team);
	}
}

/*
 * Creates a new team task with the given parameters.
 *
 * Do not create single-threaded tasks with this
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::TeamTaskData*
MixedModeSchedulerPlace<Pheet, StealingDequeT>::create_team_task(DequeItem di) {
	TeamTaskData* team_task;

	pheet_assert(di.team_size != 1);

	if(!team_task_reclamation_queue.empty() && team_task_reclamation_queue.front()->executed_countdown == 0) {
		TeamTaskData* tmp = team_task_reclamation_queue.front();
		team_task_reclamation_queue.pop();
		team_task = team_task_reuse[tmp->team_level];
		team_task_reuse[tmp->team_level] = tmp;
	}
	else {
		team_task = new TeamTaskData();
	}

	FinishStackElement* parent = di.finish_stack_element;
	if(parent < finish_stack || (parent >= (finish_stack + finish_stack_size))) {
		// to prevent thrashing on the parent finish block, we create a new finish block local to the coordinator
		// TODO: evaluate whether we can let the coordinator update the finish block and just do perform some checks
		// when it should be completed

		// Perform cleanup on finish stack
		empty_finish_stack();

		// Create new stack element for finish
		parent = create_non_blocking_finish_region(parent);
	}

	team_task->team_level = current_team->level;
	team_task->team_size = levels[current_team->level].size; //nt_size;
	team_task->task = di.task;
	team_task->parent = parent;
	team_task->next = NULL;
	team_task->executed_countdown = levels[current_team->level].size;

	// Make sure all changes are written before it will be published
	MEMORY_FENCE();

	return team_task;
}

/*
 * Creates a new team task with the given parameters. for solo execution
 *
 * Do not create multi-threaded tasks with this
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::TeamTaskData*
MixedModeSchedulerPlace<Pheet, StealingDequeT>::create_solo_team_task(DequeItem di) {
	pheet_assert(di.team_size == 1);

	TeamTaskData* team_task = team_task_reuse[num_levels - 1];

	FinishStackElement* parent = di.finish_stack_element;
	if(parent < finish_stack || (parent >= (finish_stack + finish_stack_size))) {
		// to prevent thrashing on the parent finish block, we create a new finish block local to the coordinator
		// TODO: evaluate whether we can let the coordinator update the finish block and just do perform some checks
		// when it should be completed

		// Perform cleanup on finish stack
		empty_finish_stack();

		// Create new stack element for finish
		parent = create_non_blocking_finish_region(parent);
	}

	team_task->task = di.task;
	team_task->parent = parent;

	return team_task;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::announce_first_team_task(TeamTaskData* team_task) {
//	performance_counters.num_tasks_at_level.incr(team_task->team_level);

	current_team->first_task = team_task;
	current_team_task = team_task;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::announce_next_team_task(TeamTaskData* team_task) {
//	performance_counters.num_tasks_at_level.incr(team_task->team_level);

	current_team_task->next = team_task;
	current_team_task = team_task;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::FinishStackElement*
MixedModeSchedulerPlace<Pheet, StealingDequeT>::create_non_blocking_finish_region(FinishStackElement* parent) {
	pheet_assert(finish_stack_filled_left < finish_stack_size);

	finish_stack[finish_stack_filled_left].num_finished_remote = 0;
	// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
	finish_stack[finish_stack_filled_left].num_spawned = 1;
	finish_stack[finish_stack_filled_left].parent = parent;

	if(finish_stack_filled_left >= finish_stack_init_left/* && stack_filled_left < stack_init_right*/) {
		finish_stack[finish_stack_filled_left].version = 0;
		++finish_stack_init_left;
	}
	else {
		++(finish_stack[finish_stack_filled_left].version);
	}
	pheet_assert((finish_stack[finish_stack_filled_left].version & 1) == 0);

	++finish_stack_filled_left;

	return &(finish_stack[finish_stack_filled_left - 1]);
}

/*
 * empty finish_stack but not below limit
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::empty_finish_stack() {
	while(finish_stack_filled_left > 0) {
		size_t se = finish_stack_filled_left - 1;
		if(finish_stack[se].num_spawned == finish_stack[se].num_finished_remote &&
				(finish_stack[se].version & 1)) {
		//	finalize_finish_stack_element(&(finish_stack[se]), finish_stack[se].parent);

			finish_stack_filled_left = se;

			// When parent is set to NULL, some thread is finalizing/has finalized this stack element (otherwise we would have an error)
			// Actually we have to check before whether parent has already been set to NULL, or we might have a race
		//	pheet_assert(finish_stack[finish_stack_filled_left].parent == NULL);

		}
		else {
			break;
		}
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::signal_task_completion(FinishStackElement* finish_stack_element) {
	FinishStackElement* parent = finish_stack_element->parent;
	size_t version = finish_stack_element->version;

	bool local = finish_stack_element >= finish_stack && (finish_stack_element < (finish_stack + finish_stack_size));
	if(local) {
		pheet_assert(finish_stack_element->num_spawned > 0);
		--(finish_stack_element->num_spawned);

		// Memory fence is unfortunately required to guarantee that some thread finalizes the finish_stack_element
		// TODO: prove that the fence is enough
		MEMORY_FENCE();
	}
	else {
		// Increment num_finished_remote of parent
		SIZET_ATOMIC_ADD(&(finish_stack_element->num_finished_remote), 1);
	}
	if(finish_stack_element->num_spawned == finish_stack_element->num_finished_remote) {
		finalize_finish_stack_element(finish_stack_element, parent, version, local);
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
inline void MixedModeSchedulerPlace<Pheet, StealingDequeT>::finalize_finish_stack_element(FinishStackElement* element, FinishStackElement* parent, size_t version, bool local) {
	if(parent != NULL) {
		// We have to check if we are local too!
		// (otherwise the owner might already have modified element, and then num_spawned might be 0)
		// Rarely happens, but it happens!
		if(local && element->num_spawned == 0) {
			// No tasks processed remotely - no need for atomic ops
		//	element->parent = NULL;
			++(element->version);
			signal_task_completion(parent);
		}
		else {
			if(SIZET_CAS(&(element->version), version, version + 1)) {
				signal_task_completion(parent);
			}
		}
	}
}

/*
 * Stealing routine for idle threads
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::visit_partners() {
	// If we still have local work, this might never terminate
	pheet_assert(!has_local_work());

	performance_counters.idle_time.start_timer();
	performance_counters.visit_partners_time.start_timer();

	Backoff bo;
	DequeItem di;
	while(true) {
		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level > 0) {
			// For all except the last level we assume num_partners > 0
			pheet_assert(levels[level].num_partners > 0);
			std::uniform_int_distribution<procs_t> n_r_gen(0, levels[level].num_partners - 1);
			procs_t next_rand = n_r_gen(this->get_rng());
			pheet_assert(next_rand < levels[level].num_partners);
			Place* partner = levels[level].partners[next_rand];
			pheet_assert(partner != this);

			TeamAnnouncement* team = find_partner_team(partner, level - 1);
			if(team != NULL) {
				// Joins the team and executes all tasks. Only returns after the team has been disbanded
				performance_counters.visit_partners_time.stop_timer();
				performance_counters.idle_time.stop_timer();

				join_team(team);
				pheet_assert(current_team == NULL);
				return;
			}

			di = steal_tasks_from_partner(partner, level);
		//	di = levels[level].partners[next_rand % levels[level].num_partners]->stealing_deque.steal();

			if(di.task != NULL) {
				performance_counters.visit_partners_time.stop_timer();
				performance_counters.idle_time.stop_timer();

				create_team(di.team_size);

				if(di.team_size == 1) {
					execute_solo_queue_task(di);
				}
				else {
					execute_queue_task(di);
				}
				return;
			}
			--level;
		}
		if(scheduler_state->current_state >= 2) {
			performance_counters.visit_partners_time.stop_timer();
			performance_counters.idle_time.stop_timer();
			return;
		}
		// We may perform the cleanup now, as we have to wait anyway (and this also makes debugging easier)
		empty_finish_stack();
		bo.backoff();
	}
}

/*
 * Stealing routine for (coordinating) threads waiting for a finish
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::visit_partners_until_finished(FinishStackElement* parent) {
	// If we still have local work, this might never terminate
	pheet_assert(!has_local_work());

	performance_counters.idle_time.start_timer();
	performance_counters.wait_for_finish_time.start_timer();

	Backoff bo;
	DequeItem di;
	while(true) {
		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level > 0) {
			// For all except the last level we assume num_partners > 0
			pheet_assert(levels[level].num_partners > 0);
			std::uniform_int_distribution<procs_t> n_r_gen(0, levels[level].num_partners - 1);
			procs_t next_rand = n_r_gen(this->get_rng());
			pheet_assert(next_rand < levels[level].num_partners);
			Place* partner = levels[level].partners[next_rand];
			pheet_assert(partner != this);

			TeamAnnouncement* team = find_partner_team(partner, level - 1);
			if(team != NULL) {
				performance_counters.idle_time.stop_timer();
				performance_counters.wait_for_finish_time.stop_timer();

				// Joins the team and executes all tasks. Only returns after the team has been disbanded
				join_team(team);
				pheet_assert(current_team == NULL);
				return;
			}

			di = steal_tasks_from_partner(partner, level);
		//	di = levels[level].partners[next_rand % levels[level].num_partners]->stealing_deque.steal();

			if(di.task != NULL) {
				performance_counters.idle_time.stop_timer();
				performance_counters.wait_for_finish_time.stop_timer();

				create_team(di.team_size);

				if(di.team_size == 1) {
					execute_solo_queue_task(di);
				}
				else {
					execute_queue_task(di);
				}
				return;
			}
			--level;
		}
		if(parent->num_finished_remote == parent->num_spawned) {
			performance_counters.idle_time.stop_timer();
			performance_counters.wait_for_finish_time.stop_timer();

			return;
		}
		// We may perform the cleanup now, as we have to wait anyway (and this also makes debugging easier)
		empty_finish_stack();
		bo.backoff();
	}
}

/*
 * Stealing routine for (coordinating) threads waiting for all threads to join a team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::visit_partners_until_synced(TeamAnnouncement* my_team_announcement) {
	procs_t min_level = my_team_announcement->level + 1;
	// If we still have local work, this might never terminate
	pheet_assert(!has_local_work(min_level));

	performance_counters.idle_time.start_timer();

	Backoff bo;
	DequeItem di;
	while(true) {
		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level >= min_level) {
			// For all except the last level we assume num_partners > 0
			pheet_assert(levels[level].num_partners > 0);
			std::uniform_int_distribution<procs_t> n_r_gen(0, levels[level].num_partners - 1);
			procs_t next_rand = n_r_gen(this->get_rng());
			pheet_assert(next_rand < levels[level].num_partners);
			Place* partner = levels[level].partners[next_rand];
			pheet_assert(partner != this);

			TeamAnnouncement* team = find_partner_team(partner, level - 1);
			if(team != NULL) {
				// Joins the other team if it wins the tie-breaking scheme and executes all tasks. Only returns after the team has been disbanded
				performance_counters.idle_time.stop_timer();
				if(tie_break_team(my_team_announcement, team)) {

					// True means the other team won (false means we are still stuck with our team)
					return;
				}
				performance_counters.idle_time.start_timer();
			}

			// includes deregistration mechanism and stealing can only work if deregistration was successful
			// and we stay registered if nothing was stolen
			di = steal_for_sync(my_team_announcement, partner, level);

			if(di.task != NULL) {
				performance_counters.sync_time.stop_timer();
				performance_counters.idle_time.stop_timer();

				create_team(di.team_size);

				if(di.team_size == 1) {
					execute_solo_queue_task(di);
				}
				else {
					execute_queue_task(di);
				}
				performance_counters.sync_time.start_timer();
				return;
			}
			--level;
		}
		if(my_team_announcement->reg.parts.a == my_team_announcement->reg.parts.r) {
			performance_counters.idle_time.stop_timer();

			return;
		}
		bo.backoff();
	}
}

/*
 * Checks if the partner has a relevant team for this thread
 * This method assumes we are not bound to a team, another more complex method (with tie-breaking, etc.) is used during team sync
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::TeamAnnouncement*
MixedModeSchedulerPlace<Pheet, StealingDequeT>::find_partner_team(Place* partner, procs_t level) {
	TeamAnnouncement* team = partner->current_team;
	if(team != NULL && team->level <= level) {
		// Make sure we haven't found an old announcement (in that case the team must already be fully synced)
		if(team->reg.parts.a == team->reg.parts.r) {
			// Old announcement - leave it
			return NULL;
		}
		return team;
	}
	return NULL;
}

/*
 * Joins the team and executes all tasks. Only returns after the team has been disbanded
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::join_team(TeamAnnouncement* team) {
	// Announce the team so it is visible to others
//	announced_teams[team_announcement_index] = team;
	current_team = team;

	register_for_team(team);
	pheet_assert(team->coordinator->levels[team->level].partners == levels[team->level].partners);

	// Calculate the team_level at which we have to drop out
/*	{
		pheet_assert(levels[team->level].local_id != team->coordinator->levels[team->level].local_id);

		Place* smaller;
		Place* larger;
		if(levels[team->level].local_id < team->coordinator->levels[team->level].local_id) {
			smaller = this;
			larger = team->coordinator;
		}
		else {
			smaller = team->coordinator;
			larger = this;
		}
		pheet_assert(smaller != larger);
		procs_t diff = larger->levels[team->level].local_id - smaller->levels[team->level].local_id;
		pheet_assert(diff > 0);
		procs_t lvl = team->level + 1;
		while(smaller->levels[lvl].local_id + diff == larger->levels[lvl].local_id) {
			++lvl;
			pheet_assert(lvl < smaller->num_levels && lvl < larger->num_levels);
		}
		max_team_level = lvl - 1;
	}*/

//	pheet_assert(current_team->level <= max_team_level);

	prepare_team_info(team);

	follow_team();

	pheet_assert(current_team == NULL);
}

/*
 * Joins the team and executes all tasks. Only returns after the team has been disbanded
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::tie_break_team(TeamAnnouncement* my_team, TeamAnnouncement* other_team) {
	if(my_team->level == other_team->level && my_team <= other_team) {
		// If both teams are the same or if they are at same level and this pointer is smaller - ignore the other
		return false;
	}
	if(my_team->level > other_team->level) {
		// Teams at higher levels are executed first
		return false;
	}
	// Tie breaking complete now switch to the new team
#ifdef PHEET_DEBUG_MODE
	bool dereg =
#endif
	deregister_from_team(my_team);
#ifdef PHEET_DEBUG_MODE
	// deregistration should never fail in this case, as my_team can't be completed if other_team wins (at least 1 thread would never join my_team)
	// TODO: This might change if we allow tasks with higher thread requirements to win over smaller ones in certain cases. Recheck it then
	pheet_assert(dereg);
#endif

	performance_counters.sync_time.stop_timer();
	join_team(other_team);
	performance_counters.sync_time.start_timer();
	pheet_assert(current_team == NULL);

	register_for_team(my_team);
	return true;
}

/*
 * Joins the team and executes all tasks. Only returns after the team has been disbanded or the task to finish has come up
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::follow_team() {
	performance_counters.queue_processing_time.start_timer();

	while(true) {
		{
			performance_counters.wait_for_coordinator_time.start_timer();
			Backoff bo;
			while(current_team->first_task == NULL) {
				bo.backoff();
			}
			performance_counters.wait_for_coordinator_time.stop_timer();
			current_team_task = current_team->first_task;
			if(current_team_task == waiting_for_finish) {
				performance_counters.queue_processing_time.stop_timer();
				return;
			}

			performance_counters.queue_processing_time.stop_timer();
			execute_team_task(current_team_task);
			performance_counters.queue_processing_time.start_timer();
		}

		while(current_team->last_task != current_team_task) {
			performance_counters.wait_for_coordinator_time.start_timer();
			Backoff bo;
			while(current_team_task->next == NULL && current_team->last_task != current_team_task) {
				bo.backoff();
			}
			performance_counters.wait_for_coordinator_time.stop_timer();
			if(current_team_task->next == NULL) {
				break;
			}
			current_team_task = current_team_task->next;
			if(current_team_task == waiting_for_finish) {
				performance_counters.queue_processing_time.stop_timer();
				return;
			}

			performance_counters.queue_processing_time.stop_timer();
			execute_team_task(current_team_task);
			performance_counters.queue_processing_time.start_timer();
		}

		{
			// We have to make sure the team is synced first
			// New team smaller: Make sure all threads joined the previous team (last chance for getting those threads in)
			// New team larger: Make sure all threads saw the previous announcement
			sync_team();

			// Wait for next team to appear
			Backoff bo;
			while(current_team->next_team == NULL) {
				bo.backoff();
			}
			// Announce new team if necessary
		/*	if(current_team->level > current_team->next_team->level) {
				announced_teams[team_announcement_index] = current_team->next_team;
			}
			else {
				announced_teams[team_announcement_index] = NULL;
			}*/

			// Update team information
			current_team = current_team->next_team;
			if(current_team->level >= num_levels ||
					(current_team->coordinator->levels[0].local_id - current_team->coordinator->levels[current_team->level].local_id)
					!= (levels[0].local_id - levels[current_team->level].local_id)) {
				current_team = NULL;
				performance_counters.queue_processing_time.stop_timer();
				break;
			}

			prepare_team_info(current_team);
		}
	}
}

/*
 * Calculates all information needed for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::prepare_team_info(TeamAnnouncement* team) {
	pheet_assert(team != NULL);
	pheet_assert(team->level != num_levels - 1);
	team_info = default_team_info;
	team_info->team_level = team->level;
	team_info->team_size = levels[team->level].size;
	if(team->coordinator->levels[team->level].reverse_ids) {
		procs_t offset = team->coordinator->levels[team->level].size - 1;
		team_info->coordinator_id = offset - team->coordinator->levels[team->level].local_id;
		team_info->local_id = offset - levels[team->level].local_id;
	}
	else {
		team_info->coordinator_id = team->coordinator->levels[team->level].local_id;
		team_info->local_id = levels[team->level].local_id;
	}
}

/*
 * Calculates all information needed for the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::prepare_solo_team_info() {
	team_info = solo_team_info;
}

/*
 * Performs a synchronization of the team
 * After sync_team it is ensured, that all threads necessary for the team are working in the team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::sync_team() {
	if(current_team->reg.parts.a != current_team->reg.parts.r) {
		performance_counters.task_time.stop_timer();
		performance_counters.sync_time.start_timer();
		wait_for_sync();
		performance_counters.sync_time.stop_timer();
		performance_counters.task_time.start_timer();
	}
}

/**
 * translate a number of threads to a level in the CPU hierarchy
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_level_for_num_threads(procs_t num_threads) {
	pheet_assert(num_threads > 0);

	if(num_threads > levels[0].size) {
		return 0;
	}
	procs_t candidate = num_levels - find_last_bit_set(num_threads);
	// With perfect binary trees (e.g. if number of processors is a power of two) this loop is never executed
	// On sane hierarchies, this loop is executed at most once
	while(levels[candidate].size < num_threads) {
		--candidate;
	}
	return candidate;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::is_coordinator() {
	return team_info->coordinator_id == team_info->local_id;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_local_id() {
	return team_info->local_id;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_coordinator_id() {
	return team_info->coordinator_id;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_global_id() {
	return levels[0].local_id;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_team_size() {
	return team_info->team_size;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
procs_t MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_max_team_size() {
	return levels[0].size;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::start_finish_region() {
	if(is_coordinator()) {
		performance_counters.num_finishes.incr();

		// Perform cleanup on left side of finish stack
		empty_finish_stack();

		pheet_assert(finish_stack_filled_left < finish_stack_filled_right);
		--finish_stack_filled_right;

		finish_stack[finish_stack_filled_right].num_finished_remote = 0;
		finish_stack[finish_stack_filled_right].num_spawned = 0;
		// Parent is not finished when this element is finished, so leave parent empty
		finish_stack[finish_stack_filled_right].parent = NULL;
		if(current_team_task != NULL) {
			finish_stack[finish_stack_filled_right].prev_el = current_team_task->parent;
			current_team_task->parent = &(finish_stack[finish_stack_filled_right]);
		}
#ifdef PHEET_DEBUG_MODE
		else {
			// Some additional safety which may ease debugging
			finish_stack[finish_stack_filled_right].prev_el = NULL;
		}
#endif
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::end_finish_region() {
	performance_counters.task_time.stop_timer();
	if(is_coordinator()) {
		pheet_assert(current_team_task->parent == &(finish_stack[finish_stack_filled_right]));
		// Store current team task
		TeamTaskData* my_team_task = current_team_task;

		if(finish_stack[finish_stack_filled_right].num_spawned > finish_stack[finish_stack_filled_right].num_finished_remote) {
			// There exist some non-executed or stolen tasks

			performance_counters.queue_processing_time.start_timer();
			// Execute the rest of the tasks in the team
			bool fin = coordinate_team_until_finished(&(finish_stack[finish_stack_filled_right]));
			performance_counters.queue_processing_time.stop_timer();

			if(!fin) {
				// Process other tasks until this task has been finished
				wait_for_finish(&(finish_stack[finish_stack_filled_right]));
			}
		}
		if(my_team_task != NULL)
			my_team_task->parent = finish_stack[finish_stack_filled_right].prev_el;

		if(current_team == NULL || current_team->level != my_team_task->team_level) {
			create_team(my_team_task->team_size);
			if(my_team_task->team_size > 1) {
				announce_first_team_task(my_team_task);
			}
		}
		else if(my_team_task->team_size > 1) {
			announce_next_team_task(my_team_task);
		}

		// Remove stack element
		++finish_stack_filled_right;
	}
	else {
		wait_for_coordinator_finish(current_team_task);
	}
	performance_counters.task_time.start_timer();
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::spawn(TaskParams&& ... params) {
	if(is_coordinator()) {
		performance_counters.num_spawns.incr();
		if(team_info->team_size == 1) {
			// Special treatment of solo tasks - less synchronization needed for switch to synchroneous calls
			procs_t level = team_info->team_level;
			if(stealing_deques[level]->get_length() >= levels[level].spawn_same_size_threshold) {
				performance_counters.num_spawns_to_call.incr();
				// There are enough tasks in our queue - make a synchroneous call instead
				call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
			}
			else {
				CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
				pheet_assert(current_team_task->parent != NULL);
				current_team_task->parent->num_spawned++;
				DequeItem di;
				di.task = task;
				di.finish_stack_element = current_team_task->parent;
				di.team_size = team_info->team_size;
				store_item_in_deque(di, level);
			}
		}
		else {
			// TODO: optimization to use call in some cases to prevent the deque from growing too large
			CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
			pheet_assert(current_team_task->parent != NULL);
			current_team_task->parent->num_spawned++;
			DequeItem di;
			di.task = task;
			di.finish_stack_element = current_team_task->parent;
			di.team_size = team_info->team_size;
			store_item_in_deque(di, team_info->team_level);
		}
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::local_spawn(TaskParams&& ... params) {
	if(team_info->team_size == 1) {
		// Use the optimized synchroneous version instead
		spawn<CallTaskType, TaskParams ...>(std::forward<TaskParams&&>(params) ...);
	}
	else {
		performance_counters.num_spawns.incr();

		// We are out of sync, so calls instead of spawns are not possible
		CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
		pheet_assert(finish_stack_filled_left > 0);
		current_team_task->parent->num_spawned++;
		DequeItem di;
		di.task = task;
		di.finish_stack_element = current_team_task->parent;
		di.team_size = current_team->team_size;
		store_item_in_deque(di, team_info->team_level);
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::call(TaskParams&& ... params) {
	if(is_coordinator()) {
		performance_counters.num_calls.incr();
//		performance_counters.num_tasks_at_level.incr(team_info->team_level);

		if(team_info->team_size > 1) {
			// TODO
			pheet_assert(true);
		}
		else {
			CallTaskType task(std::forward<TaskParams&&>(params) ...);
			task();
		}
	}
	else {
		// TODO
		pheet_assert(true);
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::finish(TaskParams&& ... params) {
	start_finish_region();

	call<CallTaskType>(std::forward<TaskParams&&>(params) ...);

	end_finish_region();
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::spawn_nt(procs_t nt_size, TaskParams&& ... params) {
	procs_t level = get_level_for_num_threads(nt_size);
	if(level == team_info->team_level) {
		// If we stay at the same level use the optimized method for same-size spawns
		spawn<CallTaskType, TaskParams ...>(std::forward<TaskParams&&>(params) ...);
	}
	else {
		// TODO: optimization to use call in some cases to prevent the finish_stack from growing too large

		// TODO: let tasks be spawned by multiple threads

		if(is_coordinator()) {
			performance_counters.num_spawns.incr();

			CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
			pheet_assert(current_team_task->parent != NULL);
			current_team_task->parent->num_spawned++;
			DequeItem di;
			di.task = task;
			di.finish_stack_element = current_team_task->parent;
			di.team_size = levels[level].size;
			store_item_in_deque(di, level);
		}
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::local_spawn_nt(procs_t nt_size, TaskParams&& ... params) {
	procs_t level = get_level_for_num_threads(nt_size);
	if(level == team_info->team_level) {
		// Use the optimized synchroneous version instead
		spawn_nt<CallTaskType, TaskParams ...>(nt_size, std::forward<TaskParams&&>(params) ...);
	}
	else {
		CallTaskType* task = new CallTaskType(std::forward<TaskParams&&>(params) ...);
		pheet_assert(finish_stack_filled_left > 0);
		current_team_task->parent.num_spawned++;
		DequeItem di;
		di.task = task;
		di.finish_stack_element = current_team_task->parent;
		di.team_size = levels[level].size;
		store_item_in_deque(di, level);
	}
}
/*
template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::call_nt(procs_t nt_size, TaskParams&& ... params) {
	// TODO
	if(is_coordinator()) {
		pheet_assert(finish_stack_filled_left > 0);

		if(team_size == 1) {
			CallTaskType task(params ...);
			call_team_task(&task);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);
			call_task(team_size, task, &(finish_stack[finish_stack_filled_left - 1]));
		}
	}
	else {
		join_coordinator_subteam(nt_size);
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
template<class CallTaskType, typename ... TaskParams>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::finish_nt(procs_t nt_size, TaskParams&& ... params) {
	start_finish_region();

	call_nt<CallTaskType>(nt_size, std::forward<TaskParams&&>(params) ...);

	end_finish_region();
}
*/

/*
 * Checks whether there is still some local work that we can execute
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::has_local_work() {
	while(lowest_level_deque != NULL) {
		if(!(*lowest_level_deque)->is_empty()) {
			return true;
		}
		if(lowest_level_deque >= highest_level_deque) {
			lowest_level_deque = NULL;
			highest_level_deque = NULL;
			return false;
		}
		++lowest_level_deque;
	}
	return false;
}

/*
 * Checks whether there is still some local work that we can execute
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::has_local_work(procs_t min_level) {
	StealingDequeT<Pheet, DequeItem>** limit_deque = stealing_deques + min_level;
	while(highest_level_deque != NULL && highest_level_deque >= limit_deque) {
		if(!(*highest_level_deque)->is_empty()) {
			return true;
		}
		if(lowest_level_deque >= highest_level_deque) {
			lowest_level_deque = NULL;
			highest_level_deque = NULL;
			return false;
		}
		--highest_level_deque;
	}
	return false;
}

/*
 * Get a task from the local queues that is suited for the current team
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::DequeItem
MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_next_team_task() {
	return (*(stealing_deques + current_team->level))->pop();
}

/*
 * Get any task from the local queues
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::DequeItem
MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_next_queue_task() {
	DequeItem ret;
	while((ret = (*highest_level_deque)->pop()).task == NULL) {
		if(highest_level_deque == lowest_level_deque) {
			lowest_level_deque = NULL;
			highest_level_deque = NULL;
			return ret;
		}
		--highest_level_deque;
	}
	return ret;
}

/*
 * Get any task from the local queues
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::DequeItem
MixedModeSchedulerPlace<Pheet, StealingDequeT>::get_next_queue_task(procs_t min_level) {
	StealingDequeT<Pheet, DequeItem>** limit_deque = stealing_deques + min_level;

	DequeItem ret;
	while((ret = (*highest_level_deque)->pop()).task == NULL) {
		if(highest_level_deque == lowest_level_deque) {
			lowest_level_deque = NULL;
			highest_level_deque = NULL;
			return ret;
		}
		--highest_level_deque;
		if(highest_level_deque < limit_deque) {
			return nullable_traits<DequeItem>::null_value;
		}
	}
	pheet_assert(ret.team_size <= this->levels[min_level].size);
//	current_deque = highest_level_deque;
	return ret;
}

/*
 * Steal tasks from the given partner, but only those where the level is larger than the given level
 * The current implementation makes some reasonable assumptions about the CPU hierarchy (expects some symmetry)
 * If those assumptions are not met, tasks might sometimes be executed with less threads than requested.
 * Other than that, it shouldn't create any other problems.
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::DequeItem
MixedModeSchedulerPlace<Pheet, StealingDequeT>::steal_tasks_from_partner(Place* partner, procs_t min_level) {
	performance_counters.num_steal_calls.incr();
//	performance_counters.num_steal_calls_per_thread.incr(get_global_id());

	StealingDequeT<Pheet, DequeItem>** phld = partner->highest_level_deque;
	if(phld == nullptr) {
		performance_counters.num_unsuccessful_steal_calls.incr();
//		performance_counters.num_unsuccessful_steal_calls_per_thread.incr(get_global_id());
		return nullable_traits<DequeItem>::null_value;
	}

	procs_t partner_level = phld - partner->stealing_deques;
	procs_t my_level = partner_level;

	if(num_levels > partner->num_levels) {
		my_level += num_levels - partner->num_levels;
	}
	else if(my_level >= num_levels) {
		my_level = num_levels - 1;
	}

	while(true) {
		while(this->levels[my_level].size < partner->levels[partner_level].size) {
			--my_level;
		}

		if(my_level < min_level) {
			break;
		}

		StealingDequeT<Pheet, DequeItem>* partner_queue = partner->stealing_deques[partner_level];

		if(!partner_queue->is_empty()) {
			if(this->levels[my_level].size == partner->levels[partner_level].size) {
				// The easy case - we can steal_push tasks
				StealingDequeT<Pheet, DequeItem>* my_queue = stealing_deques[my_level];

				// try steal
				DequeItem ret = partner_queue->steal_push(*my_queue);
				if(ret.task != NULL) {
					pheet_assert(highest_level_deque == NULL || highest_level_deque < stealing_deques + my_level);
					highest_level_deque = stealing_deques + my_level;
					if(lowest_level_deque == NULL) {
						lowest_level_deque = highest_level_deque;
					}
					return ret;
				}
			}
			else {
				// We have to be more careful here - no steal_push or we might end up having the wrong tasks in the deque
				DequeItem ret = partner_queue->steal();
				if(ret.task != NULL) {
					return ret;
				}
			}
		}
		--partner_level;
	}

//	performance_counters.num_unsuccessful_steal_calls_per_thread.incr(get_global_id());
	performance_counters.num_unsuccessful_steal_calls.incr();
	return nullable_traits<DequeItem>::null_value;
}

/*
 * Steal tasks from the given partner, but only those where the level is larger than the given level
 * The current implementation makes some reasonable assumptions about the CPU hierarchy (expects some symmetry)
 * If those assumptions are not met, tasks might sometimes be executed with less threads than requested.
 * Other than that, it shouldn't create any other problems.
 */
template <class Pheet, template <class SP, typename T> class StealingDequeT>
typename MixedModeSchedulerPlace<Pheet, StealingDequeT>::DequeItem
MixedModeSchedulerPlace<Pheet, StealingDequeT>::steal_for_sync(TeamAnnouncement* my_team, Place* partner, procs_t min_level) {
	performance_counters.num_steal_calls.incr();
//	performance_counters.num_steal_calls_per_thread.incr(get_global_id());

	StealingDequeT<Pheet, DequeItem>** phld = partner->highest_level_deque;
	if(phld == NULL) {
		performance_counters.num_unsuccessful_steal_calls.incr();
//		performance_counters.num_unsuccessful_steal_calls_per_thread.incr(get_global_id());
		return nullable_traits<DequeItem>::null_value;
	}

	procs_t partner_level = phld - partner->stealing_deques;
	procs_t my_level = partner_level;

	if(num_levels > partner->num_levels) {
		my_level += num_levels - partner->num_levels;
	}
	else if(my_level >= num_levels) {
		my_level = num_levels - 1;
	}

	while(true) {
		while(this->levels[my_level].size < partner->levels[partner_level].size) {
			--my_level;
		}

		if(my_level < min_level) {
			break;
		}

		StealingDequeT<Pheet, DequeItem>* partner_queue = partner->stealing_deques[partner_level];

		if(!partner_queue->is_empty()) {
			// try to deregister before stealing
			if(!deregister_from_team(my_team)) {
				performance_counters.num_unsuccessful_steal_calls.incr();
//				performance_counters.num_unsuccessful_steal_calls_per_thread.incr(get_global_id());
				return nullable_traits<DequeItem>::null_value;
			}

			if(this->levels[my_level].size == partner->levels[partner_level].size) {
				// The easy case - we can steal_push tasks
				StealingDequeT<Pheet, DequeItem>* my_queue = stealing_deques[my_level];

				// try steal
				DequeItem ret = partner_queue->steal_push(*my_queue);
				if(ret.task != NULL) {
					pheet_assert(highest_level_deque == NULL || highest_level_deque < stealing_deques + my_level);
					highest_level_deque = stealing_deques + my_level;
					if(lowest_level_deque == NULL) {
						lowest_level_deque = highest_level_deque;
					}
					return ret;
				}
			}
			else {
				// We have to be more careful here - no steal_push or we might end up having the wrong tasks in the deque
				DequeItem ret = partner_queue->steal();
				if(ret.task != NULL) {
					return ret;
				}
			}
			// stealing failed - reregister and continue
			register_for_team(my_team);
		}
		--partner_level;
	}

	performance_counters.num_unsuccessful_steal_calls.incr();
//	performance_counters.num_unsuccessful_steal_calls_per_thread.incr(get_global_id());
	return nullable_traits<DequeItem>::null_value;
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::store_item_in_deque(DequeItem di, procs_t level) {
	pheet_assert(di.team_size <= this->levels[level].size);
	stealing_deques[level]->push(di);
	if(lowest_level_deque == NULL) {
		lowest_level_deque = stealing_deques + level;
		highest_level_deque = lowest_level_deque;
	}
	else if(lowest_level_deque > stealing_deques + level) {
		lowest_level_deque = stealing_deques + level;
	}
	else if(highest_level_deque < stealing_deques + level) {
		highest_level_deque = stealing_deques + level;
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
void MixedModeSchedulerPlace<Pheet, StealingDequeT>::register_for_team(TeamAnnouncement* team) {
	current_team = team;

	// Registration
	Registration reg, old_reg;
	Backoff bo;
	reg.complete = team->reg.complete;
	old_reg.complete = reg.complete;
	pheet_assert(reg.parts.a != reg.parts.r);
	++reg.parts.a;
	while(!UINT64_CAS(&(team->reg.complete), old_reg.complete, reg.complete)) {
		bo.backoff();
		reg.complete = team->reg.complete;
		old_reg.complete = reg.complete;
		++reg.parts.a;
	}
}

template <class Pheet, template <class SP, typename T> class StealingDequeT>
bool MixedModeSchedulerPlace<Pheet, StealingDequeT>::deregister_from_team(TeamAnnouncement* team) {
	// Deregistration
	Registration reg, old_reg;
	Backoff bo;
	reg.complete = team->reg.complete;
	old_reg.complete = reg.complete;
	if(reg.parts.a == reg.parts.r) {
		return false;
	}
	pheet_assert(reg.parts.a != 0);
	--reg.parts.a;
	while(!UINT64_CAS(&(team->reg.complete), old_reg.complete, reg.complete)) {
		bo.backoff();
		reg.complete = team->reg.complete;
		if(reg.parts.a == reg.parts.r) {
			return false;
		}
		old_reg.complete = reg.complete;
		pheet_assert(reg.parts.a != 0);
		--reg.parts.a;
	}

	current_team = NULL;
	return true;
}

}

#endif /* BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_ */
