/*
 * Pheet.h
 *
 *  Created on: Jan 31, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef CORE_PHEET_H_
#define CORE_PHEET_H_

#include <random>
#include <functional>

#include "../settings.h"
#include "SystemModelEnv.h"
#include "PrimitivesEnv.h"
#include "DataStructuresEnv.h"
#include "ConcurrentDataStructures.h"

namespace pheet {

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
class PheetEnv {
private:
	typedef PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT> Self;
	typedef PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT> ThisType;

public:
	typedef SystemModelT<Self> SystemModel;
	typedef PrimitivesT<Self> Primitives;
	typedef DataStructuresT<Self> DataStructures;
	typedef DataStructures DS;
	typedef ConcurrentDataStructuresT<Self> ConcurrentDataStructures;
	typedef ConcurrentDataStructures CDS;

	typedef typename SystemModel::MachineModel MachineModel;

	typedef typename Primitives::Backoff Backoff;
	typedef typename Primitives::Barrier Barrier;
	typedef typename Primitives::Finisher Finisher;
	typedef typename Primitives::Mutex Mutex;
	typedef typename Primitives::LockGuard LockGuard;

	typedef SchedulerT<Self> Scheduler;
	typedef Scheduler Environment;
	typedef typename Scheduler::Place Place;
	typedef typename Scheduler::Task Task;
	template <typename F>
		using FunctorTask = typename Scheduler::template FunctorTask<F>;
	typedef typename Scheduler::Finish Finish;

	template<template <class P> class NewSched>
	using WithScheduler = PheetEnv<NewSched, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>;

	template<template <class P> class NewMM>
	using WithMachineModel = PheetEnv<SchedulerT, SystemModel::template WithMachineModel<NewMM>::template BT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>;

	template<template <class P> class NewCDS>
	using WithCDS = PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, NewCDS>;

	template<template <class P, typename> class T>
	using WithStealingDeque = WithCDS<CDS::template WithStealingDeque<T>::template BT>;

	template<template <class P, typename> class NewTS>
	using WithTaskStorage = PheetEnv<Scheduler::template WithTaskStorage<NewTS>::template BT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>;

	template<template <class P, typename, template <class, class> class> class NewTS>
	using WithStrategyTaskStorage = PheetEnv<Scheduler::template WithStrategyTaskStorage<NewTS>::template BT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>;

	template<template <class P, typename, typename> class NewTS>
	using WithPriorityTaskStorage = PheetEnv<Scheduler::template WithPriorityTaskStorage<NewTS>::template BT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>;

	template <template <class> class NewPrim>
	using WithPrimitives = PheetEnv<SchedulerT, SystemModelT, NewPrim, DataStructuresT, ConcurrentDataStructuresT>;

	template<template <class> class M>
	using WithMutex = WithPrimitives<Primitives::template WithMutex<M>::template BT>;

	template<template <class> class M>
	using WithBackoff = WithPrimitives<Primitives::template WithBackoff<M>::template BT>;

	PheetEnv() {}
	~PheetEnv() {}

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
		static void spawn_s(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		static void spawn_s(Strategy s, F&& f, TaskParams&& ... params);
/*
	template<class CallTaskType, class Strategy, typename ... TaskParams>
		static void spawn_s(Strategy&& s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		static void spawn_s(Strategy&& s, F&& f, TaskParams&& ... params);
*/
	template<class CallTaskType, class Strategy, typename ... TaskParams>
		static void spawn_prio(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		static void spawn_prio(Strategy s, F&& f, TaskParams&& ... params);

	static std::mt19937& get_rng();
	template <typename IntT> static IntT rand_int(IntT max);
	template <typename IntT> static IntT rand_int(IntT min, IntT max);
	static Place* get_place();
	static procs_t get_place_id();
	static procs_t get_num_places() {
		return Scheduler::get()->get_num_places();
	}
	static Place* get_place_at(procs_t place_id) {
		return Scheduler::get()->get_place_at(place_id);
	}
	template <typename T> static T& place_singleton();

private:

};


template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::finish(TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<typename F, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::finish(F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn(TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<typename F, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_s(Strategy s, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->template spawn_s<CallTaskType>(std::move(s), std::forward<TaskParams&&>(params) ...);
}
/*
template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_s(Strategy&& s, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->spawn_s<CallTaskType>(std::forward<Strategy&&>(s), std::forward<TaskParams&&>(params) ...);
}
*/
template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class Strategy, typename F, typename ... TaskParams>
inline void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_s(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::move(s), f, std::forward<TaskParams&&>(params) ...);
}
/*
template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class Strategy, typename F, typename ... TaskParams>
inline void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_s(Strategy&& s, F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::forward<Strategy&&>(s), f, std::forward<TaskParams&&>(params) ...);
}*/

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, class Strategy, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_prio(Strategy s, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->template spawn_prio<CallTaskType>(s, std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class Strategy, typename F, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::spawn_prio(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->spawn_prio(s, f, std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<class CallTaskType, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::call(TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template<typename F, typename ... TaskParams>
void PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::call(F&& f, TaskParams&& ... params) {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
inline std::mt19937& PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::get_rng() {
	Place* p = Scheduler::get_place();
	pheet_assert(p != NULL);
	return p->get_rng();
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
inline
typename PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::Place*
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::get_place() {
	return Scheduler::get_place();
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
inline
procs_t
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::get_place_id() {
	Place* place = get_place();
	if(place == nullptr) {
		return 0;
	}
	return place->get_id();
}
/*
template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
inline
procs_t
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::get_num_places() {
	return Scheduler::get_num_places();
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
inline
typename PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::Place*
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::get_place_at(procs_t place_id) {
	return Scheduler::get_place_at(place_id);
}
*/
template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template <typename IntT>
inline
IntT
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::rand_int(IntT max) {
	std::uniform_int_distribution<IntT> dist(0, max);
	return dist(get_rng());
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template <typename IntT>
inline
IntT
PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::rand_int(IntT min, IntT max) {
	std::uniform_int_distribution<IntT> dist(min, max);
	return dist(get_rng());
}

template <template <class Env> class SchedulerT, template <class Env> class SystemModelT, template <class Env> class PrimitivesT, template <class Env> class DataStructuresT, template <class Env> class ConcurrentDataStructuresT>
template <typename T>
inline
T& PheetEnv<SchedulerT, SystemModelT, PrimitivesT, DataStructuresT, ConcurrentDataStructuresT>::place_singleton() {
	return Scheduler::get_place()->template singleton<T>();
}

}

#include "../sched/BStrategy/BStrategyScheduler.h"

namespace pheet {
typedef PheetEnv<BStrategyScheduler, SystemModel, Primitives, DataStructures, ConcurrentDataStructures> Pheet;

}

#endif /* CORE_PHEET_H_ */
