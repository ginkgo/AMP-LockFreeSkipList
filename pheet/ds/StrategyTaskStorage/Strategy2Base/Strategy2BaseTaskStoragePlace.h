/*
 * Strategy2BaseTaskStoragePlace.h
 *
 *  Created on: 12.08.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGY2BASETASKSTORAGEPLACE_H_
#define STRATEGY2BASETASKSTORAGEPLACE_H_

#include "Strategy2BaseTaskStorageDataBlock.h"
#include "Strategy2BaseTaskStorageItem.h"
#include "Strategy2BaseTaskStoragePerformanceCounters.h"
#include "Strategy2BaseTaskStorageBase.h"

#include <pheet/sched/Strategy2/StrategyScheduler2BaseStrategy.h>
#include <pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h>

#include <limits>
//#include <unordered_map>
#include <tuple>

namespace pheet {

template <class Pheet, class TaskStorage, typename TT, size_t BlockSize>
class Strategy2BaseTaskStoragePlace : public Strategy2BaseTaskStorageBasePlace<Pheet> {
public:
	typedef Strategy2BaseTaskStoragePlace<Pheet, TaskStorage, TT, BlockSize> Self;

	typedef TT T;

	typedef typename TaskStorage::BaseTaskStorage BaseTaskStorage;
	typedef Strategy2BaseTaskStorageBasePlace<Pheet> BaseTaskStoragePlace;

	typedef Strategy2BaseTaskStorageDataBlock<Pheet, Self, BaseTaskStorage, TT, BlockSize> DataBlock;
	typedef Strategy2BaseTaskStorageBaseItem<Pheet, BaseTaskStorage, TT> BaseItem;
	typedef Strategy2BaseTaskStorageItem<Pheet, BaseTaskStorage, DataBlock, TT> Item;

	typedef Strategy2BaseTaskStoragePerformanceCounters<Pheet>
			PerformanceCounters;

	typedef BlockItemReuseMemoryManager<Pheet, Item, Strategy2BaseTaskStorageItemReuseCheck<Item> > ItemMemoryManager;
	typedef BlockItemReuseMemoryManager<Pheet, DataBlock, Strategy2BaseTaskStorageDataBlockReuseCheck<DataBlock> > DataBlockMemoryManager;

	typedef StrategyScheduler2BaseStrategy<Pheet> BaseStrategy;

	typedef typename Pheet::Scheduler::Place SchedulerPlace;

	Strategy2BaseTaskStoragePlace(TaskStorage* task_storage, SchedulerPlace* scheduler_place, PerformanceCounters pc)
	:pc(pc), task_storage(task_storage),
	 bottom(0), top(0), phase(0), pop_in_phase(false),
	 last_partner(nullptr),
	 scheduler_place(scheduler_place)  {
		bottom_block = &(data_blocks.acquire_item());
		bottom_block->link(nullptr);
		top_block.store(bottom_block, std::memory_order_relaxed);
	//	pc.num_blocks_created.incr();

		// Full synchronization by scheduler before place becomes visible, happens-before for all initialization is satisfied
	}

	/**
	 * Only here for the compiler. Should never be called
	 */
	Strategy2BaseTaskStoragePlace(Self*) {
		throw -1;
	}

	~Strategy2BaseTaskStoragePlace() {
		pc.num_allocated_items.add(items.size());
	}

	void push(T data) {
		if(pop_in_phase) {
			// Update phase if we had a pop and then a subsequent push
			phase.store(phase.load(std::memory_order_relaxed) + 1, std::memory_order_release);
			pop_in_phase = false;
		}
		Item& it = items.acquire_item();

		size_t b = bottom.load(std::memory_order_relaxed);

//		it.strategy_v = std::move(s);
	//	it.place = this;
		it.data = data;
		it.task_storage = task_storage;
	//	it.item_push = &Self::template item_push<Strategy>;
	//	it.block = local_tail;
	//	it.offset = b;

		it.taken.store(false, std::memory_order_release);

		// If item does not fit in existing block, add a new block
		if(!bottom_block->fits(b)) {
			// First check if we already created a next block
			DataBlock* next = bottom_block->get_next();
			if(next == nullptr) {
				// Create one if not
				next = &(data_blocks.acquire_item());
				next->link(bottom_block);
			}
			bottom_block = next;
		}

		// Put item in block at position b
		bottom_block->put(&it, b);

		// If the new value of bottom is visible, the put operation must have happened before
		// All writes to item happened before as well
		bottom.store(b + 1, std::memory_order_release);
	}

	template <class Strategy>
	void push(Strategy&&, T data) {
		push(data);
	}

	void push(BaseItem* item) {
		if(pop_in_phase) {
			phase.store(phase.load(std::memory_order_relaxed) + 1, std::memory_order_release);
			pop_in_phase = false;
		}
		size_t b = bottom.load(std::memory_order_relaxed);
		pheet_assert(((ptrdiff_t)(b - top.load(std::memory_order_relaxed))) >= 0);

		// If item does not fit in existing block, add a new block
		if(!bottom_block->fits(b)) {
			// First check if we already created a next block
			DataBlock* next = bottom_block->get_next();
			if(next == nullptr) {
				// Create one if not
				next = &(data_blocks.acquire_item());
				next->link(bottom_block);
			}
			bottom_block = next;
		}
	/*	pheet_assert(item_locations.find(item) == item_locations.end());
		std::tuple<DataBlock*, size_t>& il = item_locations[item];
		std::get<0>(il) = bottom_block;
		std::get<1>(il) = b;*/
//		item_locations.put(item, std::tuple<DataBlock*, size_t>(bottom_block, b));

//		item->version.store(item->version.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);

		// Put item in block at position b
		bottom_block->put(item, b);

		// Need to do a release in this case since otherwise garbage might be seen
		// (wrong next pointers, wrong pointers to items, etc.)
		// If the new value of bottom is visible, the put operation must have happened before
		bottom.store(b + 1, std::memory_order_release);
	}

	/*
	 * Pops tasks in LIFO order. Tasks stored in a specialized task storage may allow
	 * other tasks with higher priority to be executed before them, thereby violating the
	 * LIFO order.
	 */
	T pop() {
		size_t b = bottom.load(std::memory_order_relaxed);
		size_t t = top.load(std::memory_order_relaxed);
		pheet_assert(((ptrdiff_t)(b - t)) >= 0);
		DataBlock* db = bottom_block;

		BaseItem* ret;
		while(true) {
			if(b == t) {
				// Empty, first make sure top and bottom are equal, then try stealing
				b = bottom.load(std::memory_order_relaxed);
				// If we saw some taken tasks on the way, b might be smaller than bottom.
				// Since top might have been updated in the meantime we cannot just store
				// b into bottom or top might overtake bottom
				// Instead we update top to the old value of bottom
				while(t != b) {
					// Make sure top has not overtaken bottom
					pheet_assert(((ptrdiff_t)(b - t)) > 0);
					// Make sure all tasks in this range have really been taken
					// Assertion not needed any more (may trigger in cases where it's irrelevant)
			//		pheet_assert(all_taken_local(b, t));
					// If we fail, some other thread will succeed
					// If afterwards still t != b we can just CAS again
					// TODO: Should be ok to changes this to a simple assignment, as in the dissertation, but first need to check if this is really safe
					top.compare_exchange_weak(t, b, std::memory_order_acq_rel);
				}

				return steal();
			}
			--b;

			// Did we empty current block? If yes go to predecessor
			if(!db->fits(b)) {
				db = db->get_prev();
				pheet_assert(db != nullptr);
				pheet_assert(db->fits(b));
			}

			bool valid;
			ret = db->get(b, valid);
			// Has item been taken? (Due to stealing or due to other execution orders for tasks
			// with different strategies) If yes, skip task and continue
			if(!valid || ret->taken.load(std::memory_order_acquire)) {
				continue;
			}

			// Check whether task is managed by other task storage
			if(ret->task_storage != task_storage) {
				size_t b2 = bottom.load(std::memory_order_relaxed);
				// Make sure bottom is corrected before trying to pop task
				if(b2 != (b + 1)) {
					// If task_storage->pop has at least one release statement on success,
					// this statement can be relaxed completely
					// Proof: write of new value for bottom happens before acquire of taken item
					// If item was not taken, top cannot overtake bottom
					// (On failure of pop, bottom is reset, no danger there, nothing
					// to steal anyway, but some thread may change top, so bottom needs to be reset)
					bottom.store(b + 1, std::memory_order_relaxed);
					pop_in_phase = true;
				}

				// Try popping ret or some higher priority task from the sub task-storage
				// Semantics: Other task may only be popped if ret is still not taken!!!
				T ret_data = ret->task_storage->pop(ret, scheduler_place->get_id());

				if(ret_data != nullable_traits<T>::null_value) {
					// Bottom does not need to be reset, since the task we took is guaranteed
					// to have blocked progress of top pointer before we updated bottom
					bottom_block = db;
					return ret_data;
				}

				// Make sure we don't encounter a reused variant of this item later again
//				db->put(nullptr, b);

				// Need to reset bottom to make sure that top cannot overtake bottom
				// (would be dangerous for push)
				// Stealing threads may still see bottom < top for a short time but this is ok
				// (will just lead to a failed steal attempt, and queue is empty in this case anyway)
				if(b2 != (b + 1)) {
					bottom.store(b2, std::memory_order_relaxed);
				}

				continue;
			}

			break;
		}

		bottom_block = db;

		// Now we update bottom sequentially consistent
		bottom.store(b, std::memory_order_seq_cst);
		// No need to check for taken again, this item is guaranteed to be managed by this task storage,
		// therefore safety checks are based on top and bottom

		// Sequentially consistent ordering due to bottom.store(b, seq_cst) before
		t = top.load(std::memory_order_relaxed);

		ptrdiff_t diff = (ptrdiff_t)(b - t);

		if(diff > 0) {
			// As with Arora deques, due to sequential consistency of writes to top and bottom
			// it is safe to just take the item
			T ret_data = ret->data;
			pheet_assert(!ret->taken.load(std::memory_order_relaxed));
			ret->taken.store(true, std::memory_order_release);
			return ret_data;
		}
		else if(diff == 0) {
			// As in Arora deque, all threads compete for the last available task
			// All will try to set top to t + 1

			if(top.compare_exchange_strong(t, t + 1, std::memory_order_relaxed)) {
				bottom.store(b + 1, std::memory_order_relaxed);
				T ret_data = ret->data;
				pheet_assert(!ret->taken.load(std::memory_order_relaxed));
				ret->taken.store(true, std::memory_order_release);
				return ret_data;
			}
			else {
				// Top might have been changed by more than one, so just store
				// the newest value of t (the deque is now empty, so t won't change again)
				bottom.store(t, std::memory_order_relaxed);
			}
		}
		else {
			// Since writes to top by other threads are always sequentially consistent
			// and they are sequentially consistent with our previous write to bottom
			// top cannot have been updated since we read it after our last write to bottom
			// since top > bottom and all tasks there have already been taken before
			// so we can safely set bottom to match top
			bottom.store(t, std::memory_order_relaxed);
		}
		return steal();
	}

	bool is_full() const {
		return false;
	}

	TaskStorage* get_central_task_storage() {
		return task_storage;
	}

	/**
	 * Only safe for owner of TaskStorage. For other places sometimes top > bottom may become visible
	 * leading to strange values for size.
	 */
	size_t size() const {
		return bottom.load(std::memory_order_relaxed) - top.load(std::memory_order_relaxed);
	}

	void clean_up() {
		// No need for cleanup, everything is automatically cleaned up in destructor
	}

PerformanceCounters pc;
private:
	/*
	 * Required for an assertion to make sure all tasks in this range are really taken
	 */
/*	bool all_taken_local(size_t b, size_t t) {
		pheet_assert(((ptrdiff_t)(b - t)) >= 0);
		DataBlock* db = bottom_block;

		while(b != t) {
			--b;

			// Did we empty current block? If yes go to predecessor
			if(!db->fits(b)) {
				db = db->get_prev();
				pheet_assert(db != nullptr);
				pheet_assert(db->fits(b));
			}

			bool valid;
			BaseItem* ret = db->get(b, valid);
			// Has item been taken? (Due to stealing or due to other execution orders for tasks
			// with different strategies) If yes, skip task and continue
			if(valid && !ret->taken.load(std::memory_order_relaxed)) {
				return false;
			}
		}
		return true;
	}*/

	/*
	 * Tries to steal a single task from any other place. Places are selected semirandomly
	 * Steal attempts for tasks stored in different task storages will trigger a steal
	 * on that task storage. Steal does not give any guarantees on which tasks it will
	 * attempt to steal. It will usually be FIFO, but this can and will be violated for
	 * tasks that have different task storages
	 */
	T steal() {
		if(last_partner.load(std::memory_order_relaxed) != nullptr) {
			T ret = last_partner.load(std::memory_order_relaxed)->steal_from();
			if(ret != nullable_traits<T>::null_value) {
				return ret;
			}
			last_partner.store(nullptr, std::memory_order_relaxed);
		}

		procs_t num_levels = scheduler_place->get_num_levels();
		// Finalize elements in stack
		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level > 0) {
			Self& partner = scheduler_place->get_random_partner_at_level(level)->get_base_task_storage();

			T ret = partner.steal_from();
			if(ret != nullable_traits<T>::null_value) {
				last_partner.store(&partner, std::memory_order_relaxed);
				return ret;
			}

			// Don't care about memory ordering, just give me some partner to check
			Self* partner_partner = partner.last_partner.load(std::memory_order_relaxed);
			if(partner_partner != nullptr && partner_partner != this) {
				ret = partner_partner->steal_from();
				if(ret != nullable_traits<T>::null_value) {
					// Found some work. Recheck this partner next time we steal
					last_partner.store(partner_partner, std::memory_order_relaxed);
					return ret;
				}
			}

			--level;
		}

		return nullable_traits<T>::null_value;
	}

	/**
	 * Executed in the context of the stealer, should only do thread-safe things
	 */
	T steal_from() {
		size_t p = phase.load(std::memory_order_acquire);

		// The only dangerous case is that we might see b > t although
		// b = t, but acquire makes sure we see the b that the last updater to t saw.
		// b=t can only be achieved by updating t so this is ordered.
		// Sometimes we may also see b<t for a short while only if the queue is empty,
		// but b will be reset to its old value later, so this is safe
		size_t t = top.load(std::memory_order_acquire);
		size_t old_t = t;

		size_t b = bottom.load(std::memory_order_relaxed);

		ptrdiff_t diff = (ptrdiff_t)(b-t);
		// Empty check
		if(diff <= 0) {
			// Nothing to steal
			return nullable_traits<T>::null_value;
		}

		DataBlock* db = get_top_block(t);

		size_t offset;
		while(true) {
			offset = db->get_block_offset();

			ptrdiff_t diff = (ptrdiff_t)(t - offset);
			if(diff < 0) {
				// Block was already reused. To keep it wait-free, no retries
				return nullable_traits<T>::null_value;
			}
			else if(diff >= (ptrdiff_t)BlockSize) {
				db = db->get_next();
				if(db == nullptr) {
					return nullable_traits<T>::null_value;
				}
				offset = db->get_block_offset();
				continue;
			}
			break;
		}

		bool valid;
		BaseItem* ret = db->direct_acquire(t - offset, valid);

		// Skip items that have been marked as taken
		while(!valid || ret->taken.load(std::memory_order_acquire)) {
			if(ret->task_storage == task_storage) {
				// If item from base task storage is taken there are no successors
				return nullable_traits<T>::null_value;
			}
			// Reload bottom, it might have changed after ret->taken was set
			size_t b = bottom.load(std::memory_order_acquire);

			// Reread item at position to omit ABA problem where bottom < t for
			// a short while, but a few pushes happened afterwards
			// Needs to be read after an acquire to bottom of course
			BaseItem* ret2 = db->direct_get(t - offset, valid);
			if(ret2 != ret) {
				// ABA problem, to stay wait-free just abort
				return nullable_traits<T>::null_value;
			}

			++t;
			ptrdiff_t diff = (ptrdiff_t)(b - t);
			if(diff <= 0) {
				// Empty now. No need to update t, other threads will do it
				// In fact, doing this by ourselves now would be dangerous
				return nullable_traits<T>::null_value;
			}

			// We need to find a new block to process
			if(t - offset >= BlockSize) {
				db = db->get_next();
				offset += BlockSize;
				if(db == nullptr || db->get_block_offset() != offset) {
					// Working on a reused block, return
					return nullable_traits<T>::null_value;
				}
			}

			ret = db->direct_acquire(t - offset, valid);
		}
		// We need to make sure block has not suddenly been reused
		// Never occured, and probably never would, but still needed to be sure
		if(db->get_block_offset() != offset) {
			return nullable_traits<T>::null_value;
		}

		// If we are skipping items we need to be sure that we are still in the same phase
		if(t != old_t && p != phase.load(std::memory_order_relaxed)) {
			// Phase changed. Might lead to an ABA problem where we skip some items removed by a pop
			// which are filled up again by a subsequent push
			return nullable_traits<T>::null_value;
		}

		if(ret->task_storage != task_storage) {
			// Take care, requires helping scheme in case for updating taken in case setting taken is not
			// atomic with marking an item as taken. Otherwise progress might be hindered
			T ret_data = ret->task_storage->steal(ret, scheduler_place->get_id());

			if(ret_data == nullable_traits<T>::null_value) {
				// Failure to steal
				// Spurious failures are allowed
				return nullable_traits<T>::null_value;
			}

			// We were successful in stealing the given task or a task after that one.
			// So we know for sure that pop will not find any tasks that are not taken before this one
			// Therefore we can correct the top pointer to the position before ret (ret might not yet be taken)
			if(t - old_t > 16 && p == phase.load(std::memory_order_relaxed)) {
				// We do not do this every time to reduce congestion
				// 16 was chosen arbitrarily and may be tuned

				size_t old_t2 = top.load(std::memory_order_relaxed);
				// Only do this if top has not changed in the meantime
				if(old_t2 == old_t) {
					// Weak is enough, and if we fail it's no problem, it's just a lazy correction of top anyway
					top.compare_exchange_weak(old_t, t, std::memory_order_relaxed, std::memory_order_relaxed);
				}
			}

			return ret_data;
		}

		if(t != old_t) {
			// If we skipped some items we need to update top before we do anything else
			// We allow for spurious failures, so weak is enough
			if(!top.compare_exchange_weak(old_t, t, std::memory_order_seq_cst, std::memory_order_relaxed)) {
				return nullable_traits<T>::null_value;
			}

			// Reload bottom to check if we can still increment t
			b = bottom.load(std::memory_order_relaxed);

			ptrdiff_t diff = (ptrdiff_t)(b-t);
			if(diff <= 0) {
				// Empty now
				return nullable_traits<T>::null_value;
			}
		}

		T ret_data = ret->data;

		// Steal is allowed to spuriously fail so weak is enough
		if(top.compare_exchange_weak(old_t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
			pheet_assert(!ret->taken.load(std::memory_order_relaxed));
			ret->taken.store(true, std::memory_order_release);
			return ret_data;
		}
		return nullable_traits<T>::null_value;
	}

	/**
	 * Corrects the top block pointer if necessary, and marks old blocks as reusable
	 * Can be called by any thread
	 * May only be called on non-empty queues, otherwise there may be a corner-case
	 * where t moved past the block, but the next block has not yet been initialized
	 * For this reason t has to be passed, so that the t is used for which the
	 * non-empty check is performed (linearized to there)
	 */
	DataBlock* get_top_block(size_t t) {
		DataBlock* db = top_block.load(std::memory_order_acquire);

		if(!db->fits(t)) {
			// Only one thread may update the top block, we use a TASLock for that
			// Is not blocking progress of other threads, only blocking progress for cleanup
			if(!top_block_lock.test_and_set(std::memory_order_relaxed)) {
				// Reload top_block, just in case it was changed in the meantime
				db = top_block.load(std::memory_order_acquire);

				// Update the list
				// (It is necessary to recheck if it fits before the first iteration,
				// since the top block might have changed)
				while(db->compare(t) == 1) {
					DataBlock* next = db->get_next();
					pheet_assert(next != nullptr);
					pheet_assert(((ptrdiff_t)(t - next->get_block_offset())) >= 0);
					top_block.store(next, std::memory_order_release);
					// Has release semantics
					db->mark_reusable();
					db = next;
				}
				// Free lock, so other threads can update top_block now
				top_block_lock.clear(std::memory_order_release);

				return db;
			}
			else {
				// Let some other thread do the clean-up and just iterate through the blocks
				return db;
			}
		}
		return db;
	}

	ItemMemoryManager items;
	DataBlockMemoryManager data_blocks;

	TaskStorage* task_storage;
//	StrategyRetriever sr;

	// Arora-like top and bottom. Used for local tasks
	std::atomic<size_t> bottom;
	std::atomic<size_t> top;
	// Every time a push is called after a pop a new phase is started
	// Top is not allowed to be updated if the phase changed in the meantime
	std::atomic<size_t> phase;
	bool pop_in_phase;

	DataBlock* bottom_block;
	std::atomic<DataBlock*> top_block;
	std::atomic_flag top_block_lock;

	std::atomic<Self*> last_partner;

	SchedulerPlace* scheduler_place;
};

} /* namespace pheet */
#endif /* STRATEGY2BASETASKSTORAGEPLACE_H_ */
