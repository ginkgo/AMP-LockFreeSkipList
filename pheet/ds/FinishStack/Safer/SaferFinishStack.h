/*
 * SaferFinishStack.h
 *
 *  Created on: Jul 30, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SAFERFINISHSTACK_H_
#define SAFERFINISHSTACK_H_

#include <atomic>
#include <vector>

#include "SaferFinishStackPerformanceCounters.h"

namespace pheet {

struct SaferFinishStackElement {
	// Modified by local thread. Incremented when task is spawned, decremented when finished
	std::atomic<size_t> num_spawned;

	// Only modified by other threads. Stolen tasks that were finished (including spawned tasks)
	std::atomic<size_t> num_finished_remote;

	// Pointer to num_finished_remote of another thread (the one we stole tasks from)
	SaferFinishStackElement* parent;

	// Counter to update atomically when finalizing this element (ABA problem)
	std::atomic<size_t> version;
};

/*
 * Similar to BasicFinishStack, but is less likely to run out of bounds with task execution orders
 * other than LIFO
 */
template <class Pheet, size_t StackSize>
class SaferFinishStackImpl {
public:
	typedef SaferFinishStackElement Element;
	typedef SaferFinishStackPerformanceCounters<Pheet> PerformanceCounters;

	SaferFinishStackImpl()
	: stack_filled_left(0),
	  stack_filled_right(StackSize),
	  stack_init_left(0) {

	}

	SaferFinishStackImpl(PerformanceCounters& pcs)
	: performance_counters(pcs),
	  stack_filled_left(0),
	  stack_filled_right(StackSize),
	  stack_init_left(0) {

	}

	~SaferFinishStackImpl() {

	}

	Element* create_blocking(Element* parent) {
		// All operations can be relaxed, since remote access to the new element can only be
		// established by a synchronizes-with relationship

		// Perform cleanup on left side of finish stack
		empty_stack();

		pheet_assert(stack_filled_left < stack_filled_right);

		--stack_filled_right;

		// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
		stack[stack_filled_right].num_spawned.store(1, std::memory_order_relaxed);
		stack[stack_filled_right].parent = parent;
		stack[stack_filled_right].num_finished_remote.store(0, std::memory_order_relaxed);

		performance_counters.finish_stack_blocking_min.add_value(stack_filled_right);

		return &(stack[stack_filled_right]);
	}

	Element* destroy_blocking(Element* element) {
		pheet_assert(unique(element));
		pheet_assert(element == stack + stack_filled_right);

		Element* parent = element->parent;
		++stack_filled_right;

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
		pheet_assert(l >= r);
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
		return element >= stack && (element < (stack + StackSize));
	}

	void empty_stack() {
		pheet_assert(reuse.empty());

		while(stack_filled_left > 0) {
			size_t se = stack_filled_left - 1;
			if((stack[se].version.load(std::memory_order_relaxed) & 1)) {
				stack_filled_left = se;
			}
			else {
				break;
			}
		}
	}

	void finalize(Element* element, Element* parent, size_t version, bool local) {
		if(parent != NULL) {
			// We have to check if we are local too!
			// (otherwise the owner might already have modified element, and then num_spawned might be 0)
			// Rarely happens, but it happens!

			if(local && element->num_spawned == 0) {
				pheet_assert(element >= stack && element < (stack + StackSize));
				// No tasks processed remotely - no need for atomic ops
			//	element->parent = NULL;
				// No races when writing
				element->version.store(version + 1, std::memory_order_relaxed);
				reuse.push_back(element);
				performance_counters.num_chained_completion_signals.incr();
				signal_completion(parent);
			}
			else {
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

		if(!reuse.empty()) {
			el = reuse.back();
			reuse.pop_back();

			size_t v = el->version.load(std::memory_order_relaxed);
			pheet_assert(v & 1);
			el->version.store(v + 1, std::memory_order_relaxed);
		}
		else {
			// Perform cleanup on finish stack
			empty_stack();

			pheet_assert(stack_filled_left < stack_filled_right);

			el = stack + stack_filled_left;

			if(stack_filled_left >= stack_init_left) {
				el->version.store(0, std::memory_order_relaxed);
				++stack_init_left;
			}
			else {
				size_t v = el->version.load(std::memory_order_relaxed);
				el->version.store(v + 1, std::memory_order_relaxed);
			}
			++stack_filled_left;
		}

		// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
		el->num_spawned.store(1, std::memory_order_relaxed);
		el->parent = parent;
		el->num_finished_remote.store(0, std::memory_order_relaxed);

		pheet_assert((el->version & 1) == 0);

		performance_counters.finish_stack_nonblocking_max.add_value(stack_filled_left);

		return el;
	}

	PerformanceCounters performance_counters;

	std::vector<Element*> reuse;
	Element stack[StackSize];

	size_t stack_filled_left;
	size_t stack_filled_right;
	size_t stack_init_left;
};

template <class Pheet>
using SaferFinishStack = SaferFinishStackImpl<Pheet, 8192>;

} /* namespace pheet */
#endif /* SAFERFINISHSTACK_H_ */
