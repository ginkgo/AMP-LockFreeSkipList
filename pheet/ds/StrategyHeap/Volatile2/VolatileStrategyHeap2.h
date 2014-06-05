/*
 * VolatileStrategyHeap2.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef VOLATILESTRATEGYHEAP2_H_
#define VOLATILESTRATEGYHEAP2_H_

#include "VolatileStrategyHeap2PerformanceCounters.h"
#include <limits>

namespace pheet {

class VolatileStrategySubHeap2Base {
public:
	virtual ~VolatileStrategySubHeap2Base() {}

	virtual void delete_node(void* node) = 0;
	virtual void consolidate_recursive() = 0;

	void add_child(VolatileStrategySubHeap2Base* child) {
		children.push_back(child);
	}

protected:
	std::vector<VolatileStrategySubHeap2Base*> children;
};

template <class Pheet, typename TT, class StrategyRetriever, class Strategy, class BaseStrategy>
class VolatileStrategyHeap2Node : public VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, typename Strategy::BaseStrategy, BaseStrategy>{
public:
	typedef VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, Strategy, BaseStrategy> Self;
	typedef VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, typename Strategy::BaseStrategy, BaseStrategy> Base;

	VolatileStrategyHeap2Node(TT const& data, size_t weight, VolatileStrategySubHeap2Base* base_heap)
	:Base(data, weight, base_heap), state(0), d(0) {}

	Self* prev;
	Self* next;
	Self* children;
	Self* parent; // Only a single element in the child list points to parent!!! Otherwise we need to update all nodes in child list!
	uint8_t state; // 0: not linked in, 1: linked in but not active, 2: linked in and active
	uint8_t d;
};


template <class Pheet, typename TT, class StrategyRetriever, class Strategy>
class VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, Strategy, Strategy> {
public:
	typedef VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, Strategy, Strategy> Self;

	VolatileStrategyHeap2Node(TT const& data, size_t weight, VolatileStrategySubHeap2Base* base_heap)
	:data(data), weight(weight), references(0), base_heap(base_heap), state(0), d(0) {}
	~VolatileStrategyHeap2Node() {
		pheet_assert(references == 0);
		pheet_assert(weight != 0);
		pheet_assert(weight < ((std::numeric_limits<size_t>::max() >> 3)));
	}

	TT data;
	size_t weight;
	size_t references;
	VolatileStrategySubHeap2Base* base_heap;

	Self* prev;
	Self* next;
	Self* children;
	Self* parent; // Only a single element in the child list points to parent!!! Otherwise we need to update all nodes in child list!
	uint8_t state; // 0: not linked in, 1: linked in but not active, 2: linked in and active
	uint8_t d;
};



template <class Pheet, typename TT, class StrategyRetriever, class Strategy, class BaseStrategy>
class VolatileStrategySubHeap2 : public VolatileStrategySubHeap2Base {
public:
	typedef VolatileStrategySubHeap2<Pheet, TT, StrategyRetriever, Strategy, BaseStrategy> Self;
	typedef VolatileStrategySubHeap2<Pheet, TT, StrategyRetriever, typename Strategy::BaseStrategy, BaseStrategy> Base;
	typedef VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, Strategy, BaseStrategy> Node;

	VolatileStrategySubHeap2(StrategyRetriever& sr, std::map<std::type_index, VolatileStrategySubHeap2Base*>& heap_heaps)
	:max(nullptr), sr(sr), unconsolidated_count(0) {
		VolatileStrategySubHeap2Base*& base = heap_heaps[std::type_index(typeid(typename Strategy::BaseStrategy))];
		if(base == nullptr) {
			base = new Base(sr, heap_heaps);
		}
		parent = static_cast<Base*>(base);
		parent->add_child(this);
	}
	~VolatileStrategySubHeap2() {
		if(max != nullptr) {
			cleanup_nodes(max);
		}
	}

	Strategy* get_strategy(TT const& data) {
		return reinterpret_cast<Strategy*>(sr(data));
	}

	Strategy* get_strategy(Node* node) {
		return reinterpret_cast<Strategy*>(sr(node->data));
	}

	bool create_node(TT const& data, size_t& weight) {
		Strategy* s = get_strategy(data);
		if(s == nullptr) {
			return false;
		}
		weight = s->get_transitive_weight();
		pheet_assert(weight < ((std::numeric_limits<size_t>::max() >> 3)));
		Node* new_node = new Node(data, weight, this);
		this->link_node(new_node, s);
		return true;
	}

	void link_node(Node* node, Strategy* s) {
//		pheet_assert(s != nullptr);
		if(node->state == 0) {
			++(node->references);

			if(max == nullptr) {
				max = node;
				node->prev = node;
				node->next = node;
				node->d = 0;
				node->children = nullptr;
				node->parent = nullptr;
				node->state = 1;

				parent->link_node(node, s);
			}
			else {
				max->prev->next = node;
				node->next = max;
				node->prev = max->prev;
				max->prev = node;
				node->d = 0;
				node->children = nullptr;
				node->parent = nullptr;
				node->state = 1;

				Strategy* max_s = get_strategy(max);
				if(max_s != nullptr) {
					if(s == nullptr || s->prioritize(*max_s)) {
						parent->link_node(node, s);
						if(max->state == 2) {
							sub_unconsolidated();
						}
						else {
							parent->deactivate_node(max);
						}
						max = node;
					}
				}
			}
		}
		else {
			pheet_assert(node->state == 2);
			if(node == max) {
				sub_unconsolidated();
			}
			node->state = 1;
		}
	}

	void deactivate_node(Node* node) {
		pheet_assert(node->state == 1);
		node->state = 2;
		if(node == max) {
			add_unconsolidated();
			parent->deactivate_node(max);
		}
	}

	void delete_node(void* n) {
		Node* node = reinterpret_cast<Node*>(n);
		unlink_node_recursive(node);
		delete node;
	}

	void unlink_node_recursive(Node* node) {
		if(node->state > 0) {
			unlink_node(node);
		}
		if(node->references > 0) {
			parent->unlink_node_recursive(node);
		}
	}

	void unlink_node(Node* node) {
		pheet_assert(node->state > 0);
		if(node->children != nullptr) {
			pheet_assert(node->children->parent == node);
			node->children->parent = nullptr;
			this->combine(node, node->children);
		}

		if(node->parent != nullptr) {
			pheet_assert(node->parent->children == node);
			if(node->next == node) {
				node->parent->children = nullptr;
				node->parent->d = 0;
			}
			else {
				--(node->parent->d);
				node->prev->next = node->next;
				node->next->prev = node->prev;
				node->parent->children = node->next;
				node->next->parent = node->parent;
			}
		}
		else if(max == node) {
			pheet_assert(node->parent == nullptr);
			if(node->state == 2) {
				sub_unconsolidated();
			}
			if(max->next == max) {
				max = nullptr;
			}
			else {
				node->prev->next = node->next;
				node->next->prev = node->prev;
				max = node->next;
				Strategy* s = consolidate();
				if(max != nullptr)
					parent->link_node(max, s);
			}
		}
		else {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		--(node->references);
		node->state = 0;
	}

	void add_unconsolidated() {
		++unconsolidated_count;
		if(unconsolidated_count == 1) {
			parent->add_unconsolidated();
		}
	}

	void sub_unconsolidated() {
		pheet_assert(unconsolidated_count > 0);
		--unconsolidated_count;
		if(unconsolidated_count == 0) {
			parent->sub_unconsolidated();
		}
	}

	void add_child(VolatileStrategySubHeap2Base* child) {
		children.push_back(child);
	}

	void consolidate_recursive() {
		if(unconsolidated_count > 0) {
			for(auto i = children.begin(); i != children.end(); ++i) {
				(*i)->consolidate_recursive();
			}
			if(unconsolidated_count > 0) {
				pheet_assert(unconsolidated_count == 1);
				pheet_assert(max->state == 2);
				Strategy* max_s = consolidate();
				if(max != nullptr) {
					parent->link_node(max, max_s);
				}
				sub_unconsolidated();
			}
			pheet_assert(unconsolidated_count == 0);
		}
	}

	Strategy* consolidate() {
		// Doesn't touch unconsolidated_count!!!

		pheet_assert(this->max != nullptr);

		Strategy* max_s = nullptr;

		// Since we compare with degree variable, let's use the same type
		// Otherwise gcc 4.7 gives an signed/unsigned warning for whatever reason (even if both are unsigned types)
		uint8_t init = 0;
		Node* helper[65];
		Strategy* helper_s[65];

		Node* node;
		Node* end = this->max->prev;
		Node* next = this->max;
		this->max = nullptr;
		do {
			node = next;
			next = node->next;

			// Node needs to be deleted
			if(node->state == 2) {
				if(node->children != nullptr) {
					pheet_assert(node->children->parent == node);
					node->children->parent = nullptr;
					Node* start = end->next;
					this->combine(start, node->children);
					end = start->prev;
				}
				--(node->references);
				pheet_assert(node->references > 0);
				node->state = 0;
				if(node->next == node) {
					pheet_assert(this->max == nullptr);
//					this->max = nullptr;
					return nullptr;
				}
				node->prev->next = node->next;
				node->next->prev = node->prev;
			}
			else {
				Strategy* node_s = get_strategy(node);
				if(node_s == nullptr) {
					// TODO: put those nodes in a special list and remove them from here
					this->max = node;
					return nullptr;
				}

				if(max_s == nullptr || node_s->prioritize(*max_s)) {
					this->max = node;
					max_s = node_s;
				}

				pheet_assert(node->weight != 0);
				pheet_assert(node->d < 64);
				for(; init <= node->d; ++init) {
					helper[init] = nullptr;
				}

				Node* tmp = node;
				while(helper[tmp->d] != nullptr) {
					size_t old_d = tmp->d;
					// Both tmp (from some other helper_s) and helper_s[old_d] can be max.
					if(tmp == max || node_s->prioritize(*(helper_s[old_d]))) {
						pheet_assert(helper[old_d] != this->max);
						this->combine_trees(tmp, helper[old_d]);
					}
					else {
						pheet_assert(tmp != this->max);
						this->combine_trees(helper[old_d], tmp);
						node_s = helper_s[old_d];
						tmp = helper[old_d];
					}
					helper[old_d] = nullptr;
				}
				pheet_assert(helper[tmp->d] == nullptr);
				helper[tmp->d] = tmp;
				helper_s[tmp->d] = node_s;
				if(init <= tmp->d + 1) {
					pheet_assert(init == tmp->d + 1);
					helper[init] = nullptr;
					++init;
				}
			}
		} while(node != end);

		return max_s;
	}

	void combine(Node* list1, Node* list2) {
//		Node* tmp = list1->next;
		list1->prev->next = list2;
		list2->prev->next = list1;
		std::swap(list1->prev, list2->prev);
	}

	void combine_trees(Node* larger, Node* smaller) {
		pheet_assert(smaller != max);
		pheet_assert(smaller->parent == nullptr);
		smaller->prev->next = smaller->next;
		smaller->next->prev = smaller->prev;

		if(larger->children == nullptr) {
			pheet_assert(larger->d == 0);
			larger->children = smaller;
			smaller->parent = larger;
			smaller->next = smaller;
			smaller->prev = smaller;
		}
		else {
			smaller->next = larger->children;
			smaller->prev = larger->children->prev;
			smaller->prev->next = smaller;
			smaller->next->prev = smaller;
		}
		++(larger->d);
	}
private:
	void cleanup_nodes(Node* n) {
		pheet_assert(n != nullptr);
		Node* tmp;
		Node* next = n;

		do{
			tmp = next;
			next = tmp->next;

			if(tmp->children != nullptr) {
				cleanup_nodes(tmp->children);
			}
			pheet_assert(tmp->references >= 1);
			--(tmp->references);
			if(tmp->references == 0) {
				delete tmp;
			}
		}while(next != n);
	}

	Node* max;
	StrategyRetriever& sr;
	Base* parent;
	size_t unconsolidated_count;
};

template <class Pheet, typename TT, class StrategyRetriever, class Strategy>
class VolatileStrategySubHeap2<Pheet, TT, StrategyRetriever, Strategy, Strategy> : public VolatileStrategySubHeap2Base {
public:
	typedef VolatileStrategySubHeap2<Pheet, TT, StrategyRetriever, Strategy, Strategy> Self;
	typedef VolatileStrategyHeap2Node<Pheet, TT, StrategyRetriever, Strategy, Strategy> Node;

	VolatileStrategySubHeap2(StrategyRetriever& sr, std::map<std::type_index, VolatileStrategySubHeap2Base*>&)
	:max(nullptr), sr(sr), unconsolidated_count(0) {
	}
	~VolatileStrategySubHeap2() {
		if(max != nullptr) {
			cleanup_nodes(max);
		}
	}

	Strategy* get_strategy(TT const& data) {
		return reinterpret_cast<Strategy*>(sr(data));
	}

	Strategy* get_strategy(Node* node) {
		return reinterpret_cast<Strategy*>(sr(node->data));
	}

	bool create_node(TT const& data, size_t& weight) {
		Strategy* s = sr(data);
		if(s == nullptr) {
			return false;
		}
		weight = s->get_transitive_weight();
		pheet_assert(weight < ((std::numeric_limits<size_t>::max() >> 3)));
		Node* new_node = new Node(data, weight, this);
		this->link_node(new_node, s);
		return true;
	}


	void link_node(Node* node, Strategy* s) {
		if(node->state == 0) {
			++(node->references);
		//	pheet_assert(s != nullptr);
			if(max == nullptr) {
				max = node;
				node->prev = node;
				node->next = node;
				node->d = 0;
				node->children = nullptr;
				node->parent = nullptr;
				node->state = 1;
			}
			else {
				max->prev->next = node;
				node->next = max;
				node->prev = max->prev;
				max->prev = node;
				node->d = 0;
				node->children = nullptr;
				node->parent = nullptr;
				node->state = 1;

				Strategy* max_s = get_strategy(max);
				if(max_s != nullptr) {
					if(s == nullptr || s->prioritize(*max_s)) {
						if(max->state == 2) {
							sub_unconsolidated();
						}
						max = node;
					}
				}
			}
		}
		else {
			pheet_assert(node->state == 2);
			if(node == max) {
				sub_unconsolidated();
			}
			node->state = 1;
		}
	}

	void deactivate_node(Node* node) {
		pheet_assert(node->state == 1);
		node->state = 2;
		if(node == max) {
			add_unconsolidated();
		}
	}

	void delete_node(void* n) {
		Node* node = reinterpret_cast<Node*>(n);
		unlink_node_recursive(node);
		delete node;
	}

	void unlink_node_recursive(Node* node) {
		if(node->state > 0) {
			unlink_node(node);
		}
		pheet_assert(node->references == 0);
	}

	void unlink_node(Node* node) {
		pheet_assert(node->state > 0);
		if(node->children != nullptr) {
			pheet_assert(node->children->parent == node);
			node->children->parent = nullptr;
			this->combine(node, node->children);
		}

		if(node->parent != nullptr) {
			pheet_assert(node->parent->children == node);
			if(node->next == node) {
				node->parent->d = 0;
				node->parent->children = nullptr;
			}
			else {
				--(node->parent->d);
				node->prev->next = node->next;
				node->next->prev = node->prev;
				node->parent->children = node->next;
				node->next->parent = node->parent;
			}
		}
		else if(max == node) {
			pheet_assert(node->parent == nullptr);
			if(node->state == 2) {
				sub_unconsolidated();
			}
			if(max->next == max) {
				max = nullptr;
			}
			else {
				node->prev->next = node->next;
				node->next->prev = node->prev;
				max = node->next;
				consolidate();
			}
		}
		else {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		--(node->references);
		node->state = 0;
	}

	// Only available to root heap
	TT pop_max(size_t& weight) {
		pheet_assert(max != nullptr);

		if(unconsolidated_count > 0) {
			consolidate_recursive();
		}
		pheet_assert(max != nullptr);

		TT ret = max->data;
		weight = max->weight;

		max->base_heap->delete_node(max);
		return ret;
	}

	void add_unconsolidated() {
		++unconsolidated_count;
	}

	void sub_unconsolidated() {
		pheet_assert(unconsolidated_count > 0);
		--unconsolidated_count;
	}

	void add_child(VolatileStrategySubHeap2Base* child) {
		children.push_back(child);
	}

	void consolidate_recursive() {
		if(unconsolidated_count > 0) {
			for(auto i = children.begin(); i != children.end(); ++i) {
				(*i)->consolidate_recursive();
			}
			if(unconsolidated_count > 0) {
				pheet_assert(unconsolidated_count == 1);
				pheet_assert(max->state == 2);
				consolidate();
				sub_unconsolidated();
			}
			pheet_assert(unconsolidated_count == 0);
		}
	}

	void consolidate() {
		// Doesn't touch unconsolidated_count!!!

		pheet_assert(this->max != nullptr);

		Strategy* max_s = nullptr;

		// Since we compare with degree variable, let's use the same type
		// Otherwise gcc 4.7 gives an signed/unsigned warning for whatever reason (even if both are unsigned types)
		uint8_t init = 0;
		Node* helper[65];
		Strategy* helper_s[65];

		Node* node;
		Node* end = this->max->prev;
		Node* next = this->max;
		this->max = nullptr;
		do {
			node = next;
			next = node->next;

			// Node needs to be deleted
			if(node->state == 2) {
				if(node->children != nullptr) {
					pheet_assert(node->children->parent == node);
					node->children->parent = nullptr;
					Node* start = end->next;
					this->combine(start, node->children);
					end = start->prev;
				}
				--(node->references);
				pheet_assert(node->references > 0);
				node->state = 0;
				if(node->next == node) {
					pheet_assert(this->max == nullptr);
//					this->max = nullptr;
					return;
				}
				node->prev->next = node->next;
				node->next->prev = node->prev;
			}
			else {
				Strategy* node_s = get_strategy(node);
				if(node_s == nullptr) {
					// TODO: put those nodes in a special list and remove them from here
					this->max = node;
					return;
				}

				if(max_s == nullptr || node_s->prioritize(*max_s)) {
					this->max = node;
					max_s = node_s;
				}

				pheet_assert(node->weight != 0);
				pheet_assert(node->d < 64);
				for(; init <= node->d; ++init) {
					helper[init] = nullptr;
				}

				Node* tmp = node;
				while(helper[tmp->d] != nullptr) {
					size_t old_d = tmp->d;
					// Both tmp (from some other helper_s) and helper_s[old_d] can be max.
					if(tmp == max || node_s->prioritize(*(helper_s[old_d]))) {
						pheet_assert(helper[old_d] != this->max);
						this->combine_trees(tmp, helper[old_d]);
					}
					else {
						pheet_assert(tmp != this->max);
						this->combine_trees(helper[old_d], tmp);
						node_s = helper_s[old_d];
						tmp = helper[old_d];
					}
					helper[old_d] = nullptr;
				}
				pheet_assert(helper[tmp->d] == nullptr);
				pheet_assert(tmp->parent == nullptr);
				helper[tmp->d] = tmp;
				helper_s[tmp->d] = node_s;
				if(init <= tmp->d + 1) {
					pheet_assert(init == tmp->d + 1);
					helper[init] = nullptr;
					++init;
				}
			}
		} while(node != end);

	}

	void combine(Node* list1, Node* list2) {
//		Node* tmp = list1->next;
		list1->prev->next = list2;
		list2->prev->next = list1;
		std::swap(list1->prev, list2->prev);
	}

	void combine_trees(Node* larger, Node* smaller) {
		pheet_assert(smaller != max);
		pheet_assert(smaller->parent == nullptr);
		smaller->prev->next = smaller->next;
		smaller->next->prev = smaller->prev;

		if(larger->children == nullptr) {
			pheet_assert(larger->d == 0);
			larger->children = smaller;
			smaller->parent = larger;
			smaller->next = smaller;
			smaller->prev = smaller;
		}
		else {
			smaller->next = larger->children;
			smaller->prev = larger->children->prev;
			smaller->prev->next = smaller;
			smaller->next->prev = smaller;
		}
		++(larger->d);
	}
private:
	void cleanup_nodes(Node* n) {
		pheet_assert(n != nullptr);

		Node* tmp;
		Node* next = n;

		do{
			tmp = next;
			next = tmp->next;

			if(tmp->children != nullptr) {
				cleanup_nodes(tmp->children);
			}
			pheet_assert(tmp->references >= 1);
			--(tmp->references);
			if(tmp->references == 0) {
				delete tmp;
			}
		}while(next != n);
	}

	Node* max;
	StrategyRetriever& sr;
	size_t unconsolidated_count;
};


template <class Pheet, typename TT, class StrategyRetriever>
class VolatileStrategyHeap2 {
public:
	typedef TT T;
	typedef VolatileStrategySubHeap2Base HeapBase;
//	typedef typename HeapBase::Node HeapNode;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef VolatileStrategyHeap2PerformanceCounters<Pheet> PerformanceCounters;

	template <class Strategy>
	using ItemHeap = VolatileStrategySubHeap2<Pheet, TT, StrategyRetriever, Strategy, BaseStrategy>;

	template <class Strategy>
	using HeapNode = typename ItemHeap<Strategy>::Node;

	VolatileStrategyHeap2(StrategyRetriever sr, PerformanceCounters& pc);
	~VolatileStrategyHeap2();

	template <class Strategy>
	void push(T const& item);

	T pop();
//	T& peek();

	bool empty() const { return is_empty(); }
	bool is_empty() const;
	size_t size() const { return total_size; }
	size_t transitive_weight() const { return total_weight; }

	static void print_name();

private:
	std::map<std::type_index, HeapBase*> heap_heaps;
	StrategyRetriever sr;
	ItemHeap<BaseStrategy> root_heap;
	PerformanceCounters pc;

	size_t total_size;
	size_t total_weight;
};


template <class Pheet, typename TT, class StrategyRetriever>
VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::VolatileStrategyHeap2(StrategyRetriever sr, PerformanceCounters& pc)
:sr(std::move(sr)), root_heap(sr, heap_heaps), pc(pc), total_size(0), total_weight(0) {
	heap_heaps[std::type_index(typeid(BaseStrategy))] = &root_heap;
}

template <class Pheet, typename TT, class StrategyRetriever>
VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::~VolatileStrategyHeap2() {
	for(auto i = heap_heaps.begin(); i != heap_heaps.end(); ++i) {
		if(i->second != &root_heap) {
			delete i->second;
		}
	}
}

template <class Pheet, typename TT, class StrategyRetriever>
template <class Strategy>
void VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::push(T const& item) {
	HeapBase*& bheap = heap_heaps[std::type_index(typeid(Strategy))];
	if(bheap == nullptr) {
		bheap = new ItemHeap<Strategy>(sr, heap_heaps);
	}
	ItemHeap<Strategy>* heap = static_cast<ItemHeap<Strategy>*>(bheap);
	size_t weight;
	if(heap->create_node(item, weight)) {
		++total_size;
		pheet_assert(weight != 0);
		pheet_assert(weight < ((std::numeric_limits<size_t>::max() >> 3)));
		total_weight += weight;
	}
}

template <class Pheet, typename TT, class StrategyRetriever>
TT VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::pop() {
//	total_size -= heap->perform_cleanup();
	pheet_assert(total_size > 0);

	size_t weight;
	TT ret = root_heap.pop_max(weight);

	pheet_assert(weight < ((std::numeric_limits<size_t>::max() >> 3)));
	total_weight -= weight;
	--total_size;
	return ret;
}
/*
template <class Pheet, typename TT, class StrategyRetriever>
TT& VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::peek() {
	return root_heap.peek()->data;
}*/

template <class Pheet, typename TT, class StrategyRetriever>
bool VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::is_empty() const {
	pheet_assert((total_size == 0) == (total_weight == 0));
//	pheet_assert((total_weight == 0) == root_heap.is_empty());
	return total_size == 0;
}

template <class Pheet, typename TT, class StrategyRetriever>
void VolatileStrategyHeap2<Pheet, TT, StrategyRetriever>::print_name() {
	std::cout << "VolatileStrategyHeap2";
}

} /* namespace pheet */
#endif /* VOLATILESTRATEGYHEAP2_H_ */
