/*
 * ModularTaskStorage.h
 *
 *  Created on: 21.09.2011
 *      Author: Martin Wimmer
 */

#ifndef MODULARTASKSTORAGE_H_
#define MODULARTASKSTORAGE_H_

#include <iostream>
#include "ModularTaskStoragePerformanceCounters.h"

#include "Primary/ArrayListHeap/ArrayListHeapPrimaryTaskStorage.h"
#include "Secondary/MultiSteal/MultiStealSecondaryTaskStorage.h"

namespace pheet {

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
class ModularTaskStorageImpl {
public:
	typedef ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT> Self;
	typedef TT T;
	typedef typename Pheet::Scheduler Scheduler;
	typedef PrimaryT<Pheet, T> Primary;
	typedef SecondaryT<Pheet, T, PrimaryT> Secondary;
	typedef typename Pheet::Scheduler::StealerDescriptor StealerDescriptor;
	typedef ModularTaskStoragePerformanceCounters<Pheet, typename Primary::PerformanceCounters, typename Secondary::PerformanceCounters> PerformanceCounters;

	ModularTaskStorageImpl(size_t expected_capacity);
//	ModularTaskStorageImpl(size_t expected_capacity, PerformanceCounters& perf_count);
	~ModularTaskStorageImpl();

	template <class Strategy>
	void push(Strategy& s, T item);
	template <class Strategy>
	void push(Strategy& s, T item, PerformanceCounters& pc);
	T pop();
	T pop(PerformanceCounters& pc);
	T peek();
	T peek(PerformanceCounters& pc);
	T steal(StealerDescriptor& sd);
	T steal(StealerDescriptor& sd, PerformanceCounters& pc);

	T steal_push(Self& other, StealerDescriptor& sd);
	T steal_push(Self& other, StealerDescriptor& sd, PerformanceCounters& pc);

	size_t get_length() const;
	size_t get_length(PerformanceCounters& pc) const;
	bool is_empty() const;
	bool is_empty(PerformanceCounters& pc) const;
	bool is_full() const;
	bool is_full(PerformanceCounters& pc) const;

	// Can be called by the scheduler every time it is idle to perform some routine maintenance
	void perform_maintenance(PerformanceCounters& pc);

	static void print_name();

private:
	Primary primary;
	// The destructor if the secondary structure is called first (C++ standard) - That's exactly what we want
	// (Destruction happens in reverse order of construction)
	Secondary secondary;

	PerformanceCounters perf_count;
};

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::ModularTaskStorageImpl(size_t expected_capacity)
: primary(expected_capacity), secondary(&primary, expected_capacity) {

}
/*
template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::ModularTaskStorageImpl(size_t initial_capacity, PerformanceCounters& perf_count)
: primary(initial_capacity, perf_count.primary_perf_count), secondary(&primary, perf_count.secondary_perf_count), perf_count(perf_count) {

}*/

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::~ModularTaskStorageImpl() {

}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
template <class Strategy>
inline void ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::push(Strategy& s, T item) {
	PerformanceCounters pc;
	primary.push(s, item, pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
template <class Strategy>
inline void ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::push(Strategy& s, T item, PerformanceCounters& pc) {
	primary.push(s, item, pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::pop() {
	PerformanceCounters pc;
	return primary.pop(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::pop(PerformanceCounters& pc) {
	return primary.pop(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::peek() {
	PerformanceCounters pc;
	return primary.peek(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::peek(PerformanceCounters& pc) {
	return primary.peek(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::steal(StealerDescriptor& sd) {
	PerformanceCounters pc;
	return secondary.steal(sd, pc.primary_perf_count, pc.secondary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::steal(StealerDescriptor& sd, PerformanceCounters& pc) {
	return secondary.steal(sd, pc.primary_perf_count, pc.secondary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::steal_push(Self& other, StealerDescriptor& sd) {
	// Currently we don't plan to support stealing more than one task, as this would require require reconfiguring the strategies
	PerformanceCounters pc;
	return secondary.steal_push(other.primary, sd, pc.secondary_perf_count, pc.secondary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline TT ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::steal_push(Self& other, StealerDescriptor& sd, PerformanceCounters& pc) {
	// Currently we don't plan to support stealing more than one task, as this would require require reconfiguring the strategies
	return secondary.steal_push(other.primary, sd, pc.primary_perf_count, pc.secondary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline size_t ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::get_length() const {
	PerformanceCounters pc;
	return primary.get_length(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline size_t ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::get_length(PerformanceCounters& pc) const {
	return primary.get_length(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline bool ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::is_empty() const {
	PerformanceCounters pc;
	return primary.is_empty(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline bool ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::is_empty(PerformanceCounters& pc) const {
	return primary.is_empty(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline bool ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::is_full() const {
	PerformanceCounters pc;
	return primary.is_full(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline bool ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::is_full(PerformanceCounters& pc) const {
	return primary.is_full(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
inline void ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::perform_maintenance(PerformanceCounters& pc) {
	primary.perform_maintenance(pc.primary_perf_count);
}

template <class Pheet, typename TT, template <class SP, typename S> class PrimaryT, template <class SP, typename S, template <class ES, typename Q> class P> class SecondaryT>
void ModularTaskStorageImpl<Pheet, TT, PrimaryT, SecondaryT>::print_name() {
	std::cout << "ModularTaskStorage<";
	Primary::print_name();
	std::cout << ", ";
	Secondary::print_name();
	std::cout << ">";
}

template<class Pheet, typename TT>
using ModularTaskStorage = ModularTaskStorageImpl<Pheet, TT, ArrayListHeapPrimaryTaskStorage, MultiStealSecondaryTaskStorage>;

}

#endif /* MODULARTASKSTORAGE_H_ */
