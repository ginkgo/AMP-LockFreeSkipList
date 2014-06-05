/*
 * ItemReuseMemoryManager.h
 *
 *  Created on: May 22, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef ITEMREUSEMEMORYMANAGER_H_
#define ITEMREUSEMEMORYMANAGER_H_

#include "ItemReuseMemoryManagerItem.h"

namespace pheet {

template <class Pheet, typename T, class ReuseCheck>
class ItemReuseMemoryManager {
public:
	typedef ItemReuseMemoryManagerItem<Pheet, T> Item;

	ItemReuseMemoryManager()
	: head(new Item()), tail(head) {
		head->next = head;
	}

	~ItemReuseMemoryManager() {
		Item* next = head->next;
		while(next != head) {
			Item* nnext = next->next;
			delete next;
			next = nnext;
		}
		delete head;
	}

	/*
	 * Returns an item from the memory manager.
	 * Amortized overhead O(1)
	 * Worst-case O(n) where n is the number of items
	 * managed by the memory manager
	 */
	T& acquire_item() {
		// Search for reusable overhead
		while(head->next != tail) {
			if(reuse_check(head->next->item)) {
				// Found reusable element
				head = head->next;
				// We can advance tail
				// (successful access pays for 2 unsuccessful accesses)
				tail = tail->next;
				if(tail != head) {
					tail = tail->next;
				}
				return head->item;
			}
			head = head->next;
		}

		// Too many checks. Splice in new element
		pheet_assert(head->next == tail);
		head->next = new Item();
		head->next->next = tail;
		head = head->next;
		tail = tail->next;
		return head->item;
	}

	/*
	 * Returns an item from the memory manager.
	 * Amortized overhead O(1)
	 * Worst-case O(n) where n is the number of items
	 * managed by the memory manager
	 */
	template <class S>
	T& acquire_item(S&& assign) {
		// Search for reusable overhead
		while(head->next != tail) {
			if(reuse_check(head->next->item)) {
				// Found reusable element
				head = head->next;
				// We can advance tail
				// (successful access pays for 2 unsuccessful accesses)
				tail = tail->next;
				if(tail != head) {
					tail = tail->next;
				}
				head->item = std::move(assign);
				return head->item;
			}
			head = head->next;
		}

		// Too many checks. Splice in new element
		pheet_assert(head->next == tail);
		head->next = new Item(std::move(assign));
		head->next->next = tail;
		head = head->next;
		tail = tail->next;
		return head->item;
	}

	/*
	 * Returns an item from the memory manager.
	 * Amortized overhead O(1)
	 * Worst-case O(n) where n is the number of items
	 * managed by the memory manager
	 */
	template <class S, typename ... V>
	T& acquire_item(S&& assign, V&& ... assign_more) {
		// Search for reusable overhead
		while(head->next != tail) {
			if(reuse_check(head->next->item)) {
				// Found reusable element
				head = head->next;
				// We can advance tail
				// (successful access pays for 2 unsuccessful accesses)
				tail = tail->next;
				if(tail != head) {
					tail = tail->next;
				}
				head->item = std::tuple<S, V ...>(std::move(assign), std::forward(assign_more ...));
				return head->item;
			}
			head = head->next;
		}

		// Too many checks. Splice in new element
		pheet_assert(head->next == tail);
		head->next = new Item(std::move(assign), std::forward(assign_more ...));
		head->next->next = tail;
		head = head->next;
		tail = tail->next;
		return head->item;
	}

private:
	Item* head;
	Item* tail;
	ReuseCheck reuse_check;
};

} /* namespace pheet */
#endif /* ITEMREUSEMEMORYMANAGER_H_ */
