/*
 * FibonacciSameHeap.h
 *
 *  Created on: Jun 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FIBONACCISAMEHEAP_H_
#define FIBONACCISAMEHEAP_H_

namespace pheet {

template <typename TT>
struct FibonacciSameHeapNode {
	typedef FibonacciSameHeapNode<TT> Self;

	TT data;
	Self* children;
	Self* last_same;
	Self* same;
	Self* prev;
	Self* next;
	uint8_t d;
};

template <class Pheet, typename TT, class Comparator = std::less<TT> >
class FibonacciSameHeap {
public:
	typedef FibonacciSameHeap<Pheet, TT, Comparator> Self;
	typedef TT T;
	typedef FibonacciSameHeapNode<TT> Node;

	FibonacciSameHeap()
	:max(nullptr), _size(0) {}

	~FibonacciSameHeap() {
		if(max != nullptr) {
			delete_node_list(max);
		}
	}

	void push(T item) {
		Node* tmp = new Node;
		tmp->same = nullptr;
		tmp->last_same = tmp;
		tmp->d = 0;
		tmp->data = item;
		tmp->children = nullptr;
		if(max == nullptr) {
			pheet_assert(_size == 0);
			tmp->next = tmp;
			tmp->prev = tmp;
			max = tmp;
		}
		else {
			tmp->prev = max->prev;
			max->prev->next = tmp;
			max->prev = tmp;
			tmp->next = max;

			if(is_less(max->data, item)) {
				max = tmp;
			}
		}
		++_size;
	}

	TT peek() {
		return max->data;
	}

	TT pop() {
		--_size;
		T ret = max->data;

		if(max->children != nullptr) {
			combine(max, max->children);
		}

		if(max->same != nullptr) {
			Node* same = max->same;
			same->last_same = max->last_same;
//			pheet_assert(same->next == nullptr);
//			pheet_assert(same->prev == nullptr);
			if(max->next == max) {
				same->next = same;
				same->prev = same;
			}
			else {
				same->next = max->next;
				same->prev = max->prev;
				same->prev->next = same;
				same->next->prev = same;
			}
			delete max;
			max = same;
		}
		else if(max->next != max) {
			Node* next = max->next;
			next->prev = max->prev;
			max->prev->next = next;

			delete max;
			max = next;

			consolidate();
		}
		else {
			pheet_assert(_size == 0);
			delete max;
			max = nullptr;
		}

		return ret;
	}

	void merge(Self&& other) {
		if(other.empty()) {
			return;
		}
		if(max == nullptr) {
			max = other.max;
			_size = other._size;

			other.max = nullptr;
			other._size = 0;
		}
		else {
			combine(max, other.max);
			_size += other._size;

			if(is_less(max->data, other.max->data)) {
				max = other.max;
			}

			other.max = nullptr;
			other._size = 0;
		}
	}

	bool empty() const {
		pheet_assert((max == nullptr) == (_size == 0));
		return max == nullptr;
	}
	bool is_empty() const {
		return empty();
	}
	size_t size() const {
		return _size;
	}
	size_t get_length() const {
		return _size;
	}


private:
	void delete_node_list(Node* node) {
		Node* end = node;

		do {
			if(node->children != nullptr) {
				delete_node_list(node->children);
			}
			Node *tmp = node->next;
			while(node != nullptr) {
				Node* same = node->same;
				delete node;
				node = same;
			}
			node = tmp;
		} while(node != end);
	}

	void combine(Node* list1, Node* list2) {
//		Node* tmp = list1->next;
		list1->prev->next = list2;
		list2->prev->next = list1;
		std::swap(list1->prev, list2->prev);
	}

	Node* combine_trees(Node* smaller, Node* larger) {
		pheet_assert(smaller != larger);
		pheet_assert(smaller->d == larger->d);
		if(!is_less(smaller->data, larger->data)){
			if(!is_less(larger->data, smaller->data)) {
				pheet_assert(larger->last_same->same == nullptr);
				pheet_assert(larger->last_same != smaller);
				larger->last_same->same = smaller;
				larger->last_same = smaller->last_same;

				smaller->prev->next = smaller->next;
				smaller->next->prev = smaller->prev;

				return larger;
			}
			else {
				std::swap(smaller, larger);
			}
		}
		pheet_assert(smaller != max || !is_less(smaller->data, larger->data));
		smaller->prev->next = smaller->next;
		smaller->next->prev = smaller->prev;

		if(larger->children == nullptr) {
			pheet_assert(larger->d == 0);
			larger->children = smaller;
			smaller->next = smaller;
			smaller->prev = smaller;
		}
		else {
			smaller->next = larger->children;
			smaller->prev = larger->children->prev;
			smaller->prev->next = smaller;
			larger->children->prev = smaller;
		}
		++(larger->d);
		return larger;
	}

	void consolidate() {
		pheet_assert(max != nullptr);

		size_t init = 0;
		Node* helper[65];

		Node* node;
		Node* end = max->prev;
		Node* next = max;
		do {
			node = next;
			next = node->next;
			pheet_assert(node->d < 64);
			for(; init <= node->d; ++init) {
				helper[init] = nullptr;
			}
			if(is_less(max->data, node->data)) {
				max = node;
			}
			Node* tmp = node;
			while(helper[tmp->d] != nullptr) {
			//	pheet_assert(count_nodes(max) == _size);
				size_t old_d = tmp->d;
				tmp = combine_trees(helper[tmp->d], tmp);
				if(tmp != max && !is_less(tmp->data, max->data)) {
					// In case of equality make sure that max is not part of a subtree
					max = tmp;
				}
			//	pheet_assert(count_nodes(max) == _size);
				helper[old_d] = nullptr;
			}
			pheet_assert(helper[tmp->d] == nullptr);
			helper[tmp->d] = tmp;
			for(; init <= tmp->d + 1; ++init) {
				helper[init] = nullptr;
			}
		} while(node != end);
	}

	size_t count_nodes(Node* node) {
		size_t ret = 0;
		Node* i = node;
		do {
			if(i->children != nullptr) {
				ret += count_nodes(i->children);
			}
			ret++;
			i = i->next;
		}while(i != node);
		return ret;
	}

	Node* max;
	size_t _size;
	Comparator is_less;
};


} /* namespace pheet */
#endif /* FIBONACCISAMEHEAP_H_ */
