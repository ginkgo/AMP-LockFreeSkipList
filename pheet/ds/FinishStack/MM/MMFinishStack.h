/*
 * MMFinishStack.h
 *
 *  Created on: Jul 30, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef MMFINISHSTACK_H_
#define MMFINISHSTACK_H_

#include <atomic>
#include <vector>

#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>

#include "MMFinishStackPerformanceCounters.h"

namespace pheet {

template <class Pheet>
struct MMFinishStackElement {
	MMFinishStackElement()
	: version(1), reuse_count(0) {}

	// Modified by local thread. Incremented when task is spawned, decremented when finished
	std::atomic<size_t> num_spawned;

	// Only modified by other threads. Stolen tasks that were finished (including spawned tasks)
	std::atomic<size_t> num_finished_remote;

	// Pointer to num_finished_remote of another thread (the one we stole tasks from)
	MMFinishStackElement* parent;

	// Counter to update atomically when finalizing this element (ABA problem)
	std::atomic<size_t> version;

	// Allows to reuse element if no task was spawned yet
	size_t reuse_count;

	// Owner of the element (Should be Pheet::Place* but is a problem on GCC 4.8 since Pheet::Place is not defined at this point...)
	void* /* typename Pheet::Place* */ owner;
};

template <class Element>
struct MMFinishStackElementReuseCheck {
	bool operator()(Element& element) {
		return element.version.load(std::memory_order_relaxed) & 1;
	}
};

/*
 * Similar to BasicFinishStack, but is less likely to run out of bounds with task execution orders
 * other than LIFO
 */
template <class Pheet, template <class, class, class> class MMT>
class MMFinishStackImpl {
public:
	typedef MMFinishStackElement<Pheet> Element;
	typedef MMFinishStackPerformanceCounters<Pheet> PerformanceCounters;
	typedef MMT<Pheet, Element, MMFinishStackElementReuseCheck<Element> > MM;

	MMFinishStackImpl() {

	}

	MMFinishStackImpl(PerformanceCounters& pcs)
	: performance_counters(pcs) {

	}

	~MMFinishStackImpl() {

	}

	Element* create_blocking(Element* parent) {
		// All operations can be relaxed, since remote access to the new element can only be
		// established by a synchronizes-with relationship

		// Reuse parent if possible
		if(parent != nullptr && is_local(parent) && parent->num_spawned.load(std::memory_order_relaxed) == 1) {
			pheet_assert(parent->num_finished_remote.load(std::memory_order_relaxed) == 0);
			++(parent->reuse_count);
			return parent;
		}

		Element* el;
		if(reuse.empty()) {
			el = &(mm.acquire_item());
			el->owner = Pheet::get_place();

			performance_counters.num_finish_stack_mm_accesses.incr();
		}
		else {
			el = reuse.back();
			reuse.pop_back();

			pheet_assert(is_local(el));
		}

		// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
		el->num_spawned.store(1, std::memory_order_relaxed);
		el->parent = parent;
		el->num_finished_remote.store(0, std::memory_order_relaxed);
		auto v = el->version.load(std::memory_order_relaxed);
		pheet_assert(v & 1);
		el->version.store(v + 1, std::memory_order_relaxed);

		return el;
	}

	Element* destroy_blocking(Element* element) {
		pheet_assert(unique(element));
		pheet_assert(element->owner == Pheet::get_place());

		if(element->reuse_count > 0) {
			--(element->reuse_count);
			return element;
		}

		Element* parent = element->parent;
		auto v = element->version.load(std::memory_order_relaxed);
		element->version.store(v + 1, std::memory_order_relaxed);
		reuse.push_back(element);

		return parent;
	}

	/*
	 * Only to be called by owner of element!
	 */
	void spawn(Element* element) {
		pheet_assert(is_local(element));

		size_t l = element->num_spawned.load(std::memory_order_relaxed);
		// Safe concerning signal_completion since it is read after a total ordering is established
		// using seq_cst operations
		// Safe concerning unique, since unique is also only called by the owner
		element->num_spawned.store(l + 1, std::memory_order_relaxed);
	}

	void signal_completion(Element* element) {
		// Happens before relationship between create_(non_)blocking/spawn and signal_completion
		// needs to be established externally

		performance_counters.num_completion_signals.incr();
		Element* parent = element->parent;
		// This is ok, since we can assume there is a synchronizes-with relationship between creating
		// a task that uses this element and signaling its completion
		// The rest is local ordering of atomic operations where happens-before is guaranteed
		size_t version = element->version.load(std::memory_order_relaxed);

		bool local = is_local(element);
		size_t l;
		size_t r;

		// Happens-before between store and load is locally established since they are atomic operations
		// Sequential consistency for stores in both paths is established, meaning that 1 thread is bound
		// to see both its own store, and the other store
		if(local) {
			pheet_assert(element->num_spawned.load(std::memory_order_relaxed) > 0);

			l = element->num_spawned.load(std::memory_order_relaxed) - 1;
			element->num_spawned.store(l, std::memory_order_seq_cst);

			r = element->num_finished_remote.load(std::memory_order_relaxed);
		}
		else {
			r = element->num_finished_remote.fetch_add(1, std::memory_order_seq_cst) + 1;
			l = element->num_spawned.load(std::memory_order_relaxed);
		}
		pheet_assert(l >= r || (!local && element->version.load(std::memory_order_relaxed) != version));
		if(l == r) {
			finalize(element, parent, version, local);
		}
	}

	Element* active_element(Element* element) {
		if(!is_local(element)) {
			element = create_non_blocking(element);
		}
		return element;
	}

	/*
	 * Only to be called by the owner of the element!
	 */
	bool unique(Element* element) {
		pheet_assert(is_local(element));

		// Relaxed is ok. num_finished_remote can never shrink. num_spawned is only modified locally
		return element->num_finished_remote.load(std::memory_order_relaxed) + 1
				== element->num_spawned.load(std::memory_order_relaxed);
	}

private:
	bool is_local(Element* element) {
		return element->owner == Pheet::get_place();
	}

	void finalize(Element* element, Element* parent, size_t version, bool local) {
		if(parent != NULL) {
			// We have to check if we are local too!
			// (otherwise the owner might already have modified element, and then num_spawned might be 0)
			// Rarely happens, but it happens!

			if(local && element->num_spawned == 0) {
				pheet_assert(is_local(element));
				pheet_assert(element->version.load(std::memory_order_relaxed) == version);
				// No tasks processed remotely - no need for atomic ops
			//	element->parent = NULL;
				// No races when writing
				element->version.store(version + 1, std::memory_order_relaxed);
				reuse.push_back(element);
				performance_counters.num_chained_completion_signals.incr();
				signal_completion(parent);
			}
			else if(element->version.load(std::memory_order_relaxed) == version) {
				if(element->version.compare_exchange_strong(version, version + 1, std::memory_order_release, std::memory_order_relaxed)) {
					if(local) {
						reuse.push_back(element);
					}
					performance_counters.num_chained_completion_signals.incr();
					performance_counters.num_remote_chained_completion_signals.incr();
					signal_completion(parent);
				}
			}
		}
	}

	Element* create_non_blocking(Element* parent) {
		// All operations can be relaxed, since remote access to the new element can only be
		// established by a synchronizes-with relationship

		performance_counters.num_non_blocking_finish_regions.incr();

		Element* el;

		if(reuse.empty()) {
			el = &(mm.acquire_item());
			el->owner = Pheet::get_place();

			performance_counters.num_finish_stack_mm_accesses.incr();
		}
		else {
			el = reuse.back();
			reuse.pop_back();

			pheet_assert(is_local(el));
		}

		// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
		el->num_spawned.store(1, std::memory_order_relaxed);
		el->parent = parent;
		el->num_finished_remote.store(0, std::memory_order_relaxed);

		auto v = el->version.load(std::memory_order_relaxed);
		pheet_assert(v & 1);
		el->version.store(v + 1, std::memory_order_relaxed);

		pheet_assert((el->version & 1) == 0);

		return el;
	}

	PerformanceCounters performance_counters;

	std::vector<Element*> reuse;

	MM mm;
};

template <class Pheet>
using MMFinishStack = MMFinishStackImpl<Pheet, BlockItemReuseMemoryManager>;

} /* namespace pheet */
#endif /* MMFINISHSTACK_H_ */
