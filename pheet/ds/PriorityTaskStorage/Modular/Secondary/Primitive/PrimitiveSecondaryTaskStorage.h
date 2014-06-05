/*
 * PrimitiveSecondaryTaskStorage.h
 *
 *  Created on: 21.09.2011
 *      Author: Martin Wimmer
 */

#ifndef PRIMITIVESECONDARYTASKSTORAGE_H_
#define PRIMITIVESECONDARYTASKSTORAGE_H_

#include "PrimitiveSecondaryTaskStoragePerformanceCounters.h"

namespace pheet {

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
class PrimitiveSecondaryTaskStorage {
public:
	typedef TT T;
	typedef PrimitiveSecondaryTaskStoragePerformanceCounters<Pheet> PerformanceCounters;

	PrimitiveSecondaryTaskStorage(Primary<Pheet, TT>* primary, size_t expected_capacity);
//	PrimitiveSecondaryTaskStorage(Primary<TT>* primary, PerformanceCounters& perf_count);
	~PrimitiveSecondaryTaskStorage();

	TT steal(typename Pheet::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc);
	TT steal_push(Primary<Pheet, TT>& other_primary, typename Pheet::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc);

	static void print_name();

private:
	Primary<Pheet, TT>* primary;

//	PerformanceCounters perf_count;

	static T const null_element;
};

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
TT const PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::null_element = nullable_traits<TT>::null_value;

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::PrimitiveSecondaryTaskStorage(Primary<Pheet, T>* primary, size_t expected_capacity)
: primary(primary) {

}
/*
template <class Pheet, typename TT, template <class SP, typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::PrimitiveSecondaryTaskStorage(Primary<Pheet, T>* primary, PerformanceCounters& perf_count)
: primary(primary), perf_count(perf_count) {

}*/

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::~PrimitiveSecondaryTaskStorage() {

}

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
TT PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::steal(typename Pheet::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc) {
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

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
TT PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::steal_push(Primary<Pheet, TT>& other_primary, typename Pheet::StealerDescriptor& sd, typename Primary<Pheet, TT>::PerformanceCounters& ppc, PerformanceCounters& pc) {
	return steal(sd, ppc, pc);
}

template <class Pheet, typename TT, template <class SP, typename S> class Primary>
void PrimitiveSecondaryTaskStorage<Pheet, TT, Primary>::print_name() {
	std::cout << "PrimitiveSecondaryTaskStorage";
}

}

#endif /* PRIMITIVESECONDARYTASKSTORAGE_H_ */
