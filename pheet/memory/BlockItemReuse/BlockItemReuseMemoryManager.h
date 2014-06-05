/*
 * BlockItemReuseMemoryManager.h
 *
 *  Created on: July 30, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef BLOCKITEMREUSEMEMORYMANAGER_H_
#define BLOCKITEMREUSEMEMORYMANAGER_H_

#include "BlockItemReuseMemoryManagerItem.h"

namespace pheet {

/*
 * Similar to ItemReuseMemoryManager, but allocates blocks of items instead of single items
 */
template <class Pheet, typename T, class ReuseCheck, size_t BlockSize, size_t Amortization>
class BlockItemReuseMemoryManagerImpl {
public:
	typedef BlockItemReuseMemoryManagerItem<Pheet, T, BlockSize> Item;

	template <size_t NewAmortization>
	using WithAmortization = BlockItemReuseMemoryManagerImpl<Pheet, T, ReuseCheck, BlockSize, NewAmortization>;

	BlockItemReuseMemoryManagerImpl()
	: head(new Item()), offset(0), amortized(0), total_size(BlockSize), new_block(true) {
		head->next = head;
	}

	~BlockItemReuseMemoryManagerImpl() {
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
		while(true) {
			while(offset < BlockSize) {
				if(new_block || reuse_check(head->items[offset])) {
					// Found reusable element
					++offset;
					// Successful access pays for 1 unsuccessful access
					amortized += 1 + Amortization;
					return head->items[offset - 1];
				}
				++offset;
			}
			if(amortized < BlockSize) {
				Item* tmp = new Item();
				tmp->next = head->next;
				head->next = tmp;
				head = tmp;
				total_size += BlockSize;

				new_block = true;
			}
			else {
				amortized = std::min(amortized, total_size * Amortization);
				amortized -= BlockSize;
				head = head->next;

				new_block = false;
			}
			offset = 0;
		}
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
		while(true) {
			while(offset < BlockSize) {
				if(new_block || reuse_check(head->item[offset])) {
					// Found reusable element
					++offset;
					// Successful access pays for 1 unsuccessful access
					amortized += 1 + Amortization;
					head->items[offset] = std::move(assign);
					return head->items[offset];
				}
				++offset;
			}
			if(amortized < BlockSize) {
				Item* tmp = new Item();
				tmp->next = head->next;
				head->next = tmp;
				head = tmp;
				total_size += BlockSize;

				new_block = true;
			}
			else {
				amortized = std::min(amortized, total_size * Amortization);
				amortized -= BlockSize;
				head = head->next;

				new_block = false;
			}
			offset = 0;
//			pheet_assert(total_size < 1000000);
		}
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
		while(true) {
			while(offset < BlockSize) {
				if(new_block || reuse_check(head->item[offset])) {
					// Found reusable element
					++offset;
					// Successful access pays for 1 unsuccessful access
					amortized += 1 + Amortization;
					head->items[offset] = std::tuple<S, V ...>(std::move(assign), std::forward(assign_more ...));
					return head->items[offset];
				}
				++offset;
			}
			if(amortized < BlockSize) {
				Item* tmp = new Item();
				tmp->next = head->next;
				head->next = tmp;
				head = tmp;
				total_size += BlockSize;

				new_block = true;
			}
			else {
				amortized = std::min(amortized, total_size * Amortization);
				amortized -= BlockSize;
				head = head->next;

				new_block = false;
			}
			offset = 0;
//			pheet_assert(total_size < 1000000);
		}
	}

	size_t size() {
		return total_size;
	}

private:
	Item* head;
	size_t offset;
	size_t amortized;
	size_t total_size;
	bool new_block;
	ReuseCheck reuse_check;
};

template <class Pheet, typename T, class ReuseCheck>
using BlockItemReuseMemoryManager = BlockItemReuseMemoryManagerImpl<Pheet, T, ReuseCheck, 256, 1>;

} /* namespace pheet */
#endif /* BLOCKITEMREUSEMEMORYMANAGER_H_ */
