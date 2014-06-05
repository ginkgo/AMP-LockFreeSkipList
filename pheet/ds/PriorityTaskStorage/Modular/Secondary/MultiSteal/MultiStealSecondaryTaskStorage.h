/*
 * MultiStealSecondaryTaskStorage.h
 *
 *  Created on: 07.01.2012
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef MULTISTEALSECONDARYTASKSTORAGE_H_
#define MULTISTEALSECONDARYTASKSTORAGE_H_

#include <algorithm>

#include "MultiStealSecondaryTaskStoragePerformanceCounters.h"

namespace pheet {

template <class Iter>
struct MultiStealSecondaryTaskStorageHeapElement {
	Iter iter;
	size_t steal_prio;
};

template <class HeapEl>
class MultiStealSecondaryTaskStorageComparator {
public:
	MultiStealSecondaryTaskStorageComparator() {};

	bool operator() (HeapEl const& lhs, HeapEl const& rhs) const
	{
		return lhs.steal_prio < rhs.steal_prio;
	}
};

template <class Pheet, typename TT, template <class P, typename S> class Primary>
class MultiStealSecondaryTaskStorage {
public:
	typedef TT T;
	typedef MultiStealSecondaryTaskStoragePerformanceCounters<Pheet> PerformanceCounters;
	typedef MultiStealSecondaryTaskStorageHeapElement<typename Primary<Pheet, T>::iterator> HeapElement;
	typedef MultiStealSecondaryTaskStorageComparator<HeapElement> Comparator;

	MultiStealSecondaryTaskStorage(Primary<Pheet, TT>* primary, size_t expected_capacity);
//	MultiStealSecondaryTaskStorage(Primary<TT>* primary, PerformanceCounters& perf_count);
	~MultiStealSecondaryTaskStorage();

	TT steal(typename Pheet::Scheduler::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc);
	TT steal_push(Primary<Pheet, TT>& other_primary, typename Pheet::Scheduler::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc);

	static void print_name();

private:
	Primary<Pheet, TT>* primary;
	size_t expected_capacity;

//	PerformanceCounters perf_count;

	static T const null_element;
};

template <class Pheet, typename TT, template <class P, typename S> class Primary>
TT const MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::null_element = nullable_traits<TT>::null_value;

template <class Pheet, typename TT, template <class P, typename S> class Primary>
inline MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::MultiStealSecondaryTaskStorage(Primary<Pheet, T>* primary, size_t expected_capacity)
: primary(primary), expected_capacity(expected_capacity) {

}
/*
template <class Pheet, typename TT, template <class P, typename S> class Primary>
inline MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::MultiStealSecondaryTaskStorage(Primary<Pheet, T>* primary, PerformanceCounters& perf_count)
: primary(primary), perf_count(perf_count) {

}*/

template <class Pheet, typename TT, template <class P, typename S> class Primary>
inline MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::~MultiStealSecondaryTaskStorage() {

}

template <class Pheet, typename TT, template <class P, typename S> class Primary>
TT MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::steal(typename Pheet::Scheduler::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc) {
	pc.steal_time.start_timer();
	typename Primary<Pheet, T>::iterator begin = primary->begin(ppc);
	typename Primary<Pheet, T>::iterator end = primary->end(ppc);

	// If this happens we probably have invalid iterators
	// We might change this assertion to use some number dependent on ptrdiff_t max, but for now this is better for debugging
	pheet_assert(end - begin < 0xFFFFFFF);

	pc.total_size_steal.add(end - begin);
	// g++ 4.4 unfortunately generates a compiler warning for this. Just ignore it, 4.6.1 seems to be more intelligent.
	typename Primary<Pheet, T>::iterator best;
	prio_t best_prio = 0;

	for(typename Primary<Pheet, T>::iterator i = begin; i != end; ++i) {
		if(!primary->is_taken(i, ppc)) {
			prio_t tmp_prio = primary->get_steal_priority(i, sd, ppc);
			if(tmp_prio > best_prio) {
				best = i;
				best_prio = tmp_prio;
			}
		}
	}

	if(best_prio == 0) {
		pc.steal_time.stop_timer();
		pc.num_unsuccessful_steals.incr();
		return null_element;
	}
	// We don't care if taking fails, this is just stealing, which is allowed to fail
	TT ret = primary->take(best, ppc);
	if(ret != null_element) {
		pc.num_stolen.incr();
		pc.num_successful_steals.incr();
	}
	else {
		pc.num_unsuccessful_steals.incr();
	}
	pc.steal_time.stop_timer();
	return ret;
}

template <class Pheet, typename TT, template <class P, typename S> class Primary>
TT MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::steal_push(Primary<Pheet, TT>& other_primary, typename Pheet::Scheduler::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc) {
	pc.steal_time.start_timer();
	typename Primary<Pheet, T>::iterator begin = primary->begin(ppc);
	typename Primary<Pheet, T>::iterator end = primary->end(ppc);

	// If this happens we probably have invalid iterators
	// We might change this assertion to use some number dependent on ptrdiff_t max, but for now this is better for debugging
	pheet_assert(end - begin < 0xFFFFFFF);

	pc.total_size_steal.add(end - begin);

	size_t const threshold = expected_capacity;
	typename Pheet::DS::template PriorityDeque<HeapElement, Comparator> heap;
	size_t num_dropped = 0;

	for(typename Primary<Pheet, T>::iterator i = begin; i != end; ++i) {
		if(!primary->is_taken(i, ppc)) {
			HeapElement he;
			he.steal_prio = primary->get_steal_priority(i, sd, ppc);
			if(he.steal_prio > 0) {
				if(heap.get_length() < threshold) {
					he.iter = i;
					heap.push(he);
				}
				else if(heap.min().steal_prio < he.steal_prio) {
					++num_dropped;
					he.iter = i;
					heap.replace_min(he);
				}
			}
		}
	}

	while(heap.get_length() != 0) {
		HeapElement he = heap.pop_max();

		TT ret = primary->take(he.iter, ppc);
		if(ret != null_element) {
			pc.num_stolen.incr();
			pc.num_successful_steals.incr();

			size_t to_steal = std::min((num_dropped + heap.get_length()) >> 1, heap.get_length());

			if(to_steal > 0) {
				typename Primary<Pheet, T>::iterator* steal_data = new typename Primary<Pheet, T>::iterator[to_steal];

				for(size_t i = 0; i < to_steal; ++i) {
					steal_data[i] = heap.pop_max().iter;
				}
				std::sort(steal_data, steal_data + to_steal);
				primary->transfer(other_primary, steal_data, to_steal, ppc);
				delete[] steal_data;
			}

			pc.steal_time.stop_timer();
			return ret;
		}
	}

	pc.num_unsuccessful_steals.incr();
	pc.steal_time.stop_timer();
	return null_element;
}

template <class Pheet, typename TT, template <class P, typename S> class Primary>
void MultiStealSecondaryTaskStorage<Pheet, TT, Primary>::print_name() {
	std::cout << "MultiStealSecondaryTaskStorage";
}

}

#endif /* MULTISTEALSECONDARYTASKSTORAGE_H_ */
