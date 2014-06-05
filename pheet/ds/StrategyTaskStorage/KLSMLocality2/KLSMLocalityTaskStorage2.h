/*
 * KLSMLocalityTaskStorage2.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE2_H_
#define KLSMLOCALITYTASKSTORAGE2_H_

#include "KLSMLocalityTaskStorage2Place.h"

#include <atomic>

namespace pheet {

template <class Pheet, class Strategy>
class KLSMLocalityTaskStorage2 : public Pheet::Scheduler::BaseTaskStorage {
public:
	typedef KLSMLocalityTaskStorage2<Pheet, Strategy> Self;
	typedef typename Strategy::BaseStrategy::TaskStorage ParentTaskStorage;
	typedef typename ParentTaskStorage::BaseItem BaseItem;
	typedef typename BaseItem::T T;
	typedef KLSMLocalityTaskStorage2Place<Pheet, Self, typename ParentTaskStorage::Place, Strategy> Place;

	KLSMLocalityTaskStorage2(ParentTaskStorage* parent)
	:parent(parent) {
		procs_t num_places = Pheet::get_num_places();
		places = new std::atomic<Place*>[num_places];
		for(procs_t i = 0; i < num_places; ++i) {
			places[i].store(nullptr, std::memory_order_relaxed);
		}
	}

	~KLSMLocalityTaskStorage2() {
		delete[] places;
		if(singleton.load(std::memory_order_relaxed) == this)
			singleton = nullptr;
	}

	/*
	 * Called by each place exactly once. Only one place will get created == true
	 * This place is responsible for cleanup of base task storage
	 */
	static Self* get(Place* place, ParentTaskStorage* parent, bool& created) {
		Self* ret = singleton.load(std::memory_order_acquire);
		if(ret == nullptr) {
			// We try to create a task storage
			ret = new Self(parent);

			Self* expected = nullptr;
			pheet_assert(ret != nullptr);
			if(singleton.compare_exchange_strong(expected, ret, std::memory_order_acq_rel, std::memory_order_acquire)) {
				created = true;
			}
			else {
				// Conflict on initialization, need to delete doubly initialized object
				created = false;
				delete ret;
				pheet_assert(expected != nullptr);
				pheet_assert(expected != ret);
				pheet_assert(expected == singleton.load(std::memory_order_relaxed));
				ret = expected;
			}
		}
		else {
			created = false;
		}
		ret->places[Pheet::get_place_id()].store(place, std::memory_order_release);
		return ret;
	}

	/*
	 * Override from base class
	 */
	T pop(BaseItem* boundary, procs_t place_id) {
		// Should only be called by the owner
		pheet_assert(place_id == Pheet::get_place_id());

		// Load place.
		Place* place = places[place_id].load(std::memory_order_relaxed);
		// Should already be filled, otherwise there would be no pop
		pheet_assert(place != nullptr);

		return place->pop(boundary);
	}

	/*
	 * Override from base class
	 */
	T steal(BaseItem* boundary, procs_t place_id) {
		// Should be called by everyone except the owner
		pheet_assert(place_id != Pheet::get_place_id());

		// For LSM stealing is called in the context of the stealer, therefore need to retrieve place
		// from Scheduler (might not yet have been initialized)
		Place* place = Pheet::get_place()->template get_task_storage<Strategy>();
		// Place should be created if it does not exist
		pheet_assert(place != nullptr);
		pheet_assert(place == places[Pheet::get_place_id()]);

		return place->steal(boundary);
	}

	static void print_name() {
		std::cout << "KLSMLocalityTaskStorage2";
	}

	Place* get_place(procs_t id) {
		return places[id].load(std::memory_order_acquire);
	}
private:
	static std::atomic<Self*> singleton;

	ParentTaskStorage* parent;

	std::atomic<Place*>* places;
};

template <class Pheet, class Strategy>
std::atomic<KLSMLocalityTaskStorage2<Pheet, Strategy>*>
KLSMLocalityTaskStorage2<Pheet, Strategy>::singleton(nullptr);

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE2_H_ */
