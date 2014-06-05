/*
 * PrimitivePrimaryTaskStorage.h
 *
 *  Created on: 21.09.2011
 *      Author: Martin Wimmer
 */

#ifndef PRIMITIVEPRIMARYTASKSTORAGE_H_
#define PRIMITIVEPRIMARYTASKSTORAGE_H_

#include "../../../../../settings.h"
#include "../../../../../sched/strategies/BaseStrategy.h"

#include "PrimitivePrimaryTaskStoragePerformanceCounters.h"

namespace pheet {

template <class Pheet, typename TT>
struct PrimitivePrimaryTaskStorageItem {
	TT data;
	BaseStrategy<Pheet>* s;
	prio_t pop_prio;
	prio_t steal_prio;
	size_t index;
};

template <class Pheet, typename TT, template <typename S> class CircularArray>
class PrimitivePrimaryTaskStorageImpl {
public:
	typedef TT T;
	// Not completely a standard iterator, as it doesn't support a dereference operation, but this makes implementation simpler for now (and even more lightweight)
	typedef size_t iterator;
	typedef PrimitivePrimaryTaskStoragePerformanceCounters<Pheet> PerformanceCounters;
	typedef typename Pheet::Environment::StealerDescriptor StealerDescriptor;

	PrimitivePrimaryTaskStorageImpl(size_t expected_capacity);
//	PrimitivePrimaryTaskStorageImpl(size_t expected_capacity, PerformanceCounters& perf_count);
	~PrimitivePrimaryTaskStorageImpl();

	iterator begin(PerformanceCounters& pc);
	iterator end(PerformanceCounters& pc);

	T take(iterator item, PerformanceCounters& pc);
	bool is_taken(iterator item, PerformanceCounters& pc);
	prio_t get_steal_priority(iterator item, StealerDescriptor& sd, PerformanceCounters& pc);
	size_t get_task_id(iterator item, PerformanceCounters& pc);

	template <class Strategy>
	void push(Strategy& s, T item, PerformanceCounters& pc);
	T pop(PerformanceCounters& pc);
	T peek(PerformanceCounters& pc);

	size_t get_length(PerformanceCounters& pc);
	bool is_empty(PerformanceCounters& pc);
	bool is_full(PerformanceCounters& pc);

	// Can be called by the scheduler every time it is idle to perform some routine maintenance
	void perform_maintenance(PerformanceCounters& pc);

	static void print_name();

private:
	T local_take(iterator item, PerformanceCounters& pc);
	void clean(PerformanceCounters& pc);

	size_t top;
	size_t bottom;

	CircularArray<PrimitivePrimaryTaskStorageItem<Pheet, T> > data;

//	PerformanceCounters perf_count;

	static const T null_element;
};

template <class Pheet, typename TT, template <typename S> class CircularArray>
const TT PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::null_element = nullable_traits<T>::null_value;

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::PrimitivePrimaryTaskStorageImpl(size_t expected_capacity)
: top(0), bottom(0), data(expected_capacity) {

}
/*
template <class Pheet, typename TT, template <typename S> class CircularArray>
inline PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::PrimitivePrimaryTaskStorageImpl(size_t expected_capacity, PerformanceCounters& perf_count)
: top(0), bottom(0), data(expected_capacity), perf_count(perf_count) {

}*/

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::~PrimitivePrimaryTaskStorageImpl() {

}

template <class Pheet, typename TT, template <typename S> class CircularArray>
typename PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::iterator PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::begin(PerformanceCounters& pc) {
	return top;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
typename PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::iterator PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::end(PerformanceCounters& pc) {
	return bottom;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
TT PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::local_take(iterator item, PerformanceCounters& pc) {
	pheet_assert(item < bottom);

	PrimitivePrimaryTaskStorageItem<Pheet, T>& ptsi = data.get(item);

	if(ptsi.index != item) {
		pc.num_unsuccessful_takes.incr();
		return null_element;
	}
	if(!SIZET_CAS(&(ptsi.index), item, item + 1)) {
		pc.num_unsuccessful_takes.incr();
		return null_element;
	}
	delete ptsi.s;

	pc.num_successful_takes.incr();
	return ptsi.data;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
TT PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::take(iterator item, PerformanceCounters& pc) {
	pheet_assert(item < bottom);

	PrimitivePrimaryTaskStorageItem<Pheet, T>& ptsi = data.get(item);

	if(ptsi.index != item) {
		pc.num_unsuccessful_takes.incr();
		return null_element;
	}
	// Make sure we really get the current item
	MEMORY_FENCE();

	T ret = ptsi.data;
	BaseStrategy<Pheet>* s = ptsi.s;
	if(!SIZET_CAS(&(ptsi.index), item, item + 1)) {
		pc.num_unsuccessful_takes.incr();
		return null_element;
	}
	delete s;

	pc.num_successful_takes.incr();
	return ret;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
bool PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::is_taken(iterator item, PerformanceCounters& pc) {
	return data.get(item).index != item;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
prio_t PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::get_steal_priority(iterator item, StealerDescriptor& sd, PerformanceCounters& pc) {
	return data.get(item).steal_prio;
}

/*
 * Returns a task id specifying the insertion order. some strategies use the insertion order
 * to prioritize tasks.
 * The task_id is local per queue. When a task is stolen it is assigned a new task id
 * (therefore automatically rebasing the strategy)
 * Warning! If multiple tasks are stolen and inserted into another queue, they have to be
 * inserted in the previous insertion order! (by increasing task_id)
 */
template <class Pheet, typename TT, template <typename S> class CircularArray>
size_t PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::get_task_id(iterator item, PerformanceCounters& pc) {
	return item;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
template <class Strategy>
inline void PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::push(Strategy& s, T item, PerformanceCounters& pc) {
	pc.push_time.start_timer();
	if(bottom - top == data.get_capacity()) {
		clean(pc);
		if(bottom - top == data.get_capacity()) {
			pheet_assert(data.is_growable());
			data.grow(bottom, top);
		}
	}

	PrimitivePrimaryTaskStorageItem<Pheet, TT> to_put;
	to_put.data = item;
	to_put.s = new Strategy(s);
	to_put.pop_prio = s.get_pop_priority(bottom);
	StealerDescriptor sd;
	to_put.steal_prio = s.get_steal_priority(bottom, sd);
	to_put.index = bottom;

	data.put(bottom, to_put);

	MEMORY_FENCE();

	bottom++;
	pc.push_time.stop_timer();
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline TT PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::pop(PerformanceCounters& pc) {
	pc.total_size_pop.add(bottom - top);
	pc.pop_time.start_timer();
	T ret;
	do {
		iterator i = begin(pc);
		while(i != end(pc) && is_taken(i, pc)) {
			++i;
		}

		if(i == end(pc)) {
			pc.num_unsuccessful_pops.incr();
			pc.pop_time.stop_timer();
			return null_element;
		}
		top = i;

		iterator best = i;
		prio_t best_prio = data.get(i).pop_prio;
		++i;
		for(; i != end(pc); ++i) {
			if(!is_taken(i, pc)) {
				prio_t tmp_prio = data.get(i).pop_prio;
				if(tmp_prio > best_prio) {
					best = i;
					best_prio = tmp_prio;
				}
			}
		}
		ret = local_take(best, pc);
	} while(ret == null_element);

	pc.num_successful_pops.incr();
	pc.pop_time.stop_timer();
	return ret;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline TT PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::peek(PerformanceCounters& pc) {
	iterator i = begin(pc);
	while(i != end(pc) && is_taken(top, pc)) {
		++i;
	}

	if(i == end(pc)) {
		return null_element;
	}
	top = i;

	iterator best = i;
	PrimitivePrimaryTaskStorageItem<Pheet, T> ret_item = data.get(i);
	prio_t best_prio = ret_item.pop_prio;
	++i;
	for(; i != end(pc); ++i) {
		if(!is_taken(i, pc)) {
			PrimitivePrimaryTaskStorageItem<Pheet, T> tmp_item = data.get(i);
			prio_t tmp_prio = tmp_item.pop_prio;
			if(tmp_prio > best_prio) {
				best = i;
				best_prio = tmp_prio;
				ret_item = tmp_item;
			}
		}
	}

	return ret_item.data;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline size_t PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::get_length(PerformanceCounters& pc) {
	return bottom - top;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline bool PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::is_empty(PerformanceCounters& pc) {
	clean(pc);
	return bottom == top;
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline bool PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::is_full(PerformanceCounters& pc) {
	return (!data.is_growable()) && (bottom - top == data.get_capacity());
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
void PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::clean(PerformanceCounters& pc) {
	while(top < bottom && is_taken(top, pc)) {
		++top;
	}
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
inline void PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::perform_maintenance(PerformanceCounters& pc) {
	clean(pc);
}

template <class Pheet, typename TT, template <typename S> class CircularArray>
void PrimitivePrimaryTaskStorageImpl<Pheet, TT, CircularArray>::print_name() {
	std::cout << "PrimitivePrimaryTaskStorage";
}

template <class Pheet, typename TT>
using PrimitivePrimaryTaskStorage = PrimitivePrimaryTaskStorageImpl<Pheet, TT, Pheet::CDS::template CircularArray>;

}

#endif /* PRIMITIVEPRIMARYTASKSTORAGE_H_ */
