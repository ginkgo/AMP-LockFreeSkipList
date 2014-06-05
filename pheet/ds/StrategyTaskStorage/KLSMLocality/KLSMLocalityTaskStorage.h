/*
 * KLSMLocalityTaskStorage.h
 *
 *  Created on: Sep 18, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE_H_
#define KLSMLOCALITYTASKSTORAGE_H_

#include "KLSMLocalityTaskStoragePlace.h"

#include <atomic>
#include <iostream>

namespace pheet {

template <class Pheet, class Strategy>
class KLSMLocalityTaskStorage : public Pheet::Scheduler::BaseTaskStorage {
public:
	typedef KLSMLocalityTaskStorage<Pheet, Strategy> Self;
	typedef typename Strategy::BaseStrategy::TaskStorage ParentTaskStorage;
	typedef typename ParentTaskStorage::BaseItem BaseItem;
	typedef typename BaseItem::T T;
	typedef KLSMLocalityTaskStoragePlace<Pheet, Self, typename ParentTaskStorage::Place, Strategy> Place;
	typedef typename Place::GlobalListItem GlobalListItem;

	KLSMLocalityTaskStorage(ParentTaskStorage* parent)
	:parent(parent) {
		procs_t num_places = Pheet::get_num_places();
		places = new std::atomic<Place*>[num_places];
		for(procs_t i = 0; i < num_places; ++i) {
			places[i].store(nullptr, std::memory_order_relaxed);
		}
	}

	~KLSMLocalityTaskStorage() {
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
			ret->global_list = place->create_global_list_item();

			Self* expected = nullptr;
			if(singleton.compare_exchange_strong(expected, ret, std::memory_order_acq_rel, std::memory_order_relaxed)) {
				created = true;
			}
			else {
				// Conflict on initialization, need to delete doubly initialized object
				created = false;
				delete ret;
				pheet_assert(expected != nullptr);
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
		pheet_assert(place == Pheet::get_place()->template get_task_storage<Strategy>());

		return place->pop(boundary);
	}

	/*
	 * Override from base class
	 */
	T steal(BaseItem* boundary, procs_t place_id) {
		// Should be called by everyone except the owner
		pheet_assert(place_id != Pheet::get_place_id());

		// For KLSM stealing is called in the context of the stealer, therefore need to retrieve place
		// from Scheduler (might not yet have been initialized)
		Place* place = Pheet::get_place()->template get_task_storage<Strategy>();
		// Place should be created if it does not exist
		pheet_assert(place != nullptr);
		pheet_assert(place == places[Pheet::get_place_id()]);

		return place->steal(boundary);
	}

	GlobalListItem* get_global_list_head() {
		return global_list;
	}

	static void print_name() {
		std::cout << "KLSMLocalityTaskStorage";
	}

private:
	static std::atomic<Self*> singleton;

	ParentTaskStorage* parent;

	std::atomic<Place*>* places;

	GlobalListItem* global_list;
};

template <class Pheet, class Strategy>
std::atomic<KLSMLocalityTaskStorage<Pheet, Strategy>*>
KLSMLocalityTaskStorage<Pheet, Strategy>::singleton(nullptr);

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE_H_ */
