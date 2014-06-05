/*
 * KLSMLocalityTaskStorage2BlockAnnouncement.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef KLSMLOCALITYTASKSTORAGE2BLOCKANNOUNCEMENT_H_
#define KLSMLOCALITYTASKSTORAGE2BLOCKANNOUNCEMENT_H_

#include <atomic>

namespace pheet {

template <class Pheet, class Block>
class KLSMLocalityTaskStorage2BlockAnnouncement {
public:
	typedef KLSMLocalityTaskStorage2BlockAnnouncement<Pheet, Block> Self;

	KLSMLocalityTaskStorage2BlockAnnouncement()
	: id(0), prev(nullptr), next(nullptr), block(nullptr),
	  allow_unlink(false), not_tail(false), is_head(true), processed(false),
	  unlinked(false), next_ann(nullptr), num_ann(1) {}

	~KLSMLocalityTaskStorage2BlockAnnouncement() {

	}

	void init(Block* block, Self* next_ann) {
		this->block.store(block, std::memory_order_relaxed);
		processed.store(false, std::memory_order_relaxed);
		next.store(nullptr, std::memory_order_relaxed);
		not_tail.store(false, std::memory_order_relaxed);
		is_head.store(false, std::memory_order_relaxed);
		unlinked = false;

		this->next_ann = next_ann;
		if(next_ann != nullptr) {
			num_ann = next_ann->num_ann + 1;
		} else {
			num_ann = 1;
		}
	}

	bool attempt_link(Self* ann) {
		if(next.load(std::memory_order_relaxed) != nullptr) {
			return false;
		}
		ann->prepare_link(this);
		Self* tmp = nullptr;
		return next.compare_exchange_weak(tmp, ann, std::memory_order_release, std::memory_order_acquire);
	}

	void prepare_link(Self* prev) {
		size_t _id = prev->id.load(std::memory_order_relaxed) + 1;
		if (_id & 1 == 1) {
			allow_unlink.store(true, std::memory_order_relaxed);
		}
		id.store(_id, std::memory_order_relaxed);
		this->prev.store(prev, std::memory_order_relaxed);
	}

	bool is_reusable() {
		if(unlinked &&
				(processed.load(std::memory_order_relaxed)
				|| !is_head.load(std::memory_order_relaxed))) {
			return true;
		}

		size_t _id = this->id.load(std::memory_order_relaxed);

		Self* n = next.load(std::memory_order_relaxed);
		size_t nid = n->id.load(std::memory_order_relaxed);
		if(n != nullptr && block.load(std::memory_order_relaxed) == nullptr) {
			if(unlinked) {
				if(nid != expected_next_id) {
					// Next item was already reused (ABA problem), so we can safely reuse this item
					// It can happen that an item is marked with is_head but is never processed. Those cases will be captured here
					return true;
				}

				// Try to verify that not a tail tail point to this item
				if(!not_tail.load(std::memory_order_relaxed) && n->not_tail.load(std::memory_order_relaxed)) {
					not_tail.store(true, std::memory_order_relaxed);
				}
			}
			else if(allow_unlink.load(std::memory_order_relaxed)
				&& (_id && nid) != _id) {
				// Binomial unlink
				Self* p = prev.load(std::memory_order_relaxed);

				size_t pid = p->id.load(std::memory_order_relaxed);
				pheet_assert((pid & _id) == pid);

				n->prev.store(p, std::memory_order_relaxed);
				p->next.store(n, std::memory_order_seq_cst);
				if((pid & nid) == pid) {

					// Next item will be unlinked before previous
					n->allow_unlink.store(true, std::memory_order_release);
				}
				expected_next_id = nid;
				unlinked = true;
			}
		}
		return false;
	}

	void mark_as_head() {
		is_head.store(true, std::memory_order_seq_cst);
	}

	void mark_processed() {
		processed.store(true, std::memory_order_relaxed);
	}

	void mark_as_not_tail() {
		not_tail.store(true, std::memory_order_release);
	}

	void update_block(Block* block) {
		pheet_assert(this->block.load(std::memory_order_relaxed) != nullptr);
		this->block.store(block, std::memory_order_release);
	}

	size_t get_num_announcements() {
		return num_ann;
	}

private:
	size_t id;
	std::atomic<Self*> prev;
	std::atomic<Self*> next;

	std::atomic<Block*> block;
	std::atomic<bool> allow_unlink;
	std::atomic<bool> not_tail;
	std::atomic<bool> is_head;
	std::atomic<bool> processed;

	bool unlinked;
	size_t expected_next_id;

	Self* next_ann;
	size_t num_ann;
};


template <class Item>
struct KLSMLocalityTaskStorage2BlockAnnouncementReuseCheck {
	bool operator() (Item& item) {
		return item.is_reusable();
	}
};

} /* namespace pheet */
#endif /* KLSMLOCALITYTASKSTORAGE2BLOCKANNOUNCEMENT_H_ */
