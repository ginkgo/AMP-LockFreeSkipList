/*
 * VolatileStrategyHeap.h
 *
 *  Created on: 19.06.2012
 *      Author: mwimmer
 */

#ifndef VOLATILESTRATEGYHEAP_H_
#define VOLATILESTRATEGYHEAP_H_

#include <map>
#include "VolatileStrategyHeapPerformanceCounters.h"

namespace pheet {


template <class BaseHeap, typename TT>
class VolatileStrategyHeapNode {
public:
	typedef VolatileStrategyHeapNode<BaseHeap, TT> Self;

	TT data;
	size_t weight;
	uint8_t d;
	BaseHeap* sub_heap;
	Self* children;
	Self* prev;
	Self* next;
};

template <class Pheet, typename T>
class VolatileStrategyHeapBase {
public:
	typedef VolatileStrategyHeapBase<Pheet, T> Self;
	typedef VolatileStrategyHeapNode<Self, T> Node;

	VolatileStrategyHeapBase(size_t& total_size, size_t& total_weight)
	:total_size(total_size), total_weight(total_weight), max(nullptr), parent_node(nullptr) {}

	virtual ~VolatileStrategyHeapBase() {
		if(max != nullptr) {
			delete_node_list(max);
		}
	}

//	virtual bool is_empty() = 0;
	Node* peek() {
		return max;
	}
	virtual Node* pop() = 0;


	bool empty() const {
		return max == nullptr;
	}
	bool is_empty() const {
		return empty();
	}

protected:
	void delete_node_list(Node* node) {
		Node* end = node;

		do {
			if(node->children != nullptr) {
				delete_node_list(node->children);
			}
			Node *tmp = node->next;
			delete node;
			node = tmp;
		} while(node != end);
	}

	void combine(Node* list1, Node* list2) {
//		Node* tmp = list1->next;
		list1->prev->next = list2;
		list2->prev->next = list1;
		std::swap(list1->prev, list2->prev);
	}

	void combine_trees(Node* smaller, Node* larger) {
		pheet_assert(smaller != max);
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
	}

	size_t& total_size;
	size_t& total_weight;
	Node* max;
public:
	Node* parent_node;
};

template <class Pheet, class T, class Strategy, class StrategyRetriever>
class VolatileStrategyHeapComparator {
public:
//	typedef typename Pheet::Scheduler::TaskDesc TaskDesc;
//	typedef TaskDesc<Strategy> Prep;

	VolatileStrategyHeapComparator(StrategyRetriever& sr)
	:sr(sr) {

	}

	Strategy* deref(T& t) {
		return reinterpret_cast<Strategy*>(sr(t));
	}

	void drop(T&& t) {
		sr.drop_item(std::forward<T&&>(t));
	}
/*
	bool prepare(T& t1, Prep& ) {
		return sr.prepare<Strategy>(t1, td1);
	}

	bool prepare2(T& t1) {
		return sr.prepare<Strategy>(t2, td2);
	}
*/
	bool operator()(Strategy* s1, Strategy* s2) {
		return s2->prioritize(*s1);
	}

	bool operator()(T& t1, T& t2) {
		return deref(t2)->prioritize(*deref(t1));
	}
/*
	bool try_remove(T& t) {
		if(sr.is_active(t)) {
			return false;
		}
		sr.mark_removed(t);
		return true;
	}*/

private:
	StrategyRetriever& sr;
/*	TaskDesc<Strategy> td1;
	TaskDesc<Strategy> td2;*/
};

/*
template <class Pheet, typename T>
class VolatileStrategyHeapHeapBase : public VolatileStrategyHeapBase<Pheet, T> {
public:
	typedef VolatileStrategyHeapBase<Pheet, T> Item;

	virtual void reorder_index(size_t index) = 0;
	virtual void pop_index(size_t index) = 0;

	virtual void push(Item* item) = 0;
};*/

template <class Pheet, typename T, class BaseStrategy, class Strategy, class StrategyRetriever>
class VolatileStrategyHeapHeap : public VolatileStrategyHeapBase<Pheet, T> {
public:
	typedef VolatileStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever> Self;
	typedef VolatileStrategyHeapBase<Pheet, T> Base;
	typedef VolatileStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever> Parent;
	typedef VolatileStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> Comp;
	typedef typename Base::Node Node;
//	typedef typename Comp::Prep Prep;

	VolatileStrategyHeapHeap(size_t& total_size, size_t& total_weight, StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:Base(total_size, total_weight), comp(sr), reconsolidate(false) {
		Base*& base = heap_heaps[std::type_index(typeid(typename Strategy::BaseStrategy))];
		if(base == nullptr) {
			base = new Parent(total_size, total_weight, sr, heap_heaps);
		}
		parent = static_cast<Parent*>(base);
	}

	Node* pop() {
		if(this->max == nullptr) {
			return nullptr;
		}
		if(reconsolidate) {
			consolidate();
		}
		Node* ret = this->max;

		if(this->max->children != nullptr) {
			this->combine(this->max, this->max->children);
		}

		if(this->max->sub_heap != nullptr) {
			Node* tmp = this->max->sub_heap->pop();
			if(tmp != nullptr) {
				this->combine(this->max, tmp);
			}
			else {
				this->max->sub_heap->parent_node = nullptr;
			}
		}

		if(this->max->next != this->max) {
			Node* next = this->max->next;
			next->prev = this->max->prev;
			this->max->prev->next = next;
			this->max = next;

			consolidate();
		}
		else {
//			pheet_assert(_size == 0);
//			delete max;
			this->max = nullptr;
		}
		ret->prev = ret;
		ret->next = ret;
		ret->d = 0;
		ret->children = nullptr;
		ret->sub_heap = nullptr;

		return ret;
	}

	void push(Node* node) {
		pheet_assert(node->d == 0);
		pheet_assert(node->children == nullptr);
		if(this->parent_node == nullptr) {
			node->sub_heap = this;
			parent->push(node);
			return;
		}

		Strategy* node_s = comp.deref(node->data);
		if(node_s == nullptr) {
			comp.drop(std::move(node->data));
			--(this->total_size);
			delete node;
			return;
		}
		node->weight = node_s->get_transitive_weight();
		pheet_assert(node->weight != 0);
		this->total_weight += node->weight;

		Strategy* parent_s = comp.deref(this->parent_node->data);
		if(parent_s != nullptr && (comp(node_s, parent_s))) {
			parent->swap_node(this->parent_node, node);
			node = this->parent_node;
			node_s = parent_s;
		}

		if(this->max == nullptr) {
			node->next = node;
			node->prev = node;
			this->max = node;
		}
		else {
			node->prev = this->max->prev;
			this->max->prev->next = node;
			this->max->prev = node;
			node->next = this->max;

			Strategy* max_s = comp.deref(this->max->data);
			if(max_s != nullptr && comp(node_s, max_s)) {
				this->max = node;
			}
		}

		if(node->sub_heap != nullptr) {
			pheet_assert(node->sub_heap->parent_node == nullptr);
			node->sub_heap->parent_node = node;
		}
	}

	void swap_node(Node* my_node, Node* other_node) {
		Strategy* node_s = comp.deref(other_node->data);
		pheet_assert(this->parent_node != nullptr);
		Strategy* parent_s = comp.deref(this->parent_node->data);
		if(parent_s != nullptr && comp(node_s, parent_s)) {
			parent->swap_node(this->parent_node, other_node);
			other_node = this->parent_node;
			node_s = parent_s;
		}

		if(my_node->next == my_node) {
			other_node->next = other_node;
			other_node->prev = other_node;
		}
		else {
			other_node->next = my_node->next;
			other_node->prev = my_node->prev;
			my_node->next->prev = other_node;
			my_node->prev->next = other_node;
		}
		pheet_assert(other_node->d == 0);
		other_node->children = my_node->children;

		other_node->d = my_node->d;

		Strategy* max_s = comp.deref(this->max->data);
		if(max_s != nullptr) {
			if(this->max == my_node) {
				this->max = other_node;
				// Unfortunately this might violate the ordering property, since the ordering might be different for the base strategy
				// so we need to consolidate
				if(comp(max_s, node_s)) {
					reconsolidate = true;
				}
			}
			else if(comp(node_s, max_s)) {
				this->max = other_node;
			}
		}

		std::swap(my_node->sub_heap, other_node->sub_heap);

		if(other_node->sub_heap != nullptr) {
			pheet_assert(other_node->sub_heap->parent_node == my_node);
			other_node->sub_heap->parent_node = other_node;
		}
	}

private:
	void consolidate() {
		reconsolidate = false;
		pheet_assert(this->max != nullptr);

		Strategy* max_s = comp.deref(this->max->data);
		if(max_s == nullptr) {

			return;
		}

		// Since we compare with degree variable, let's use the same type
		// Otherwise gcc 4.7 gives an signed/unsigned warning for whatever reason (even if both are unsigned types)
		uint8_t init = 0;
		Node* helper[65];
		Strategy* helper_s[65];

		Node* node;
		Node* end = this->max->prev;
		Node* next = this->max;
		do {
			node = next;
			next = node->next;
			pheet_assert(node->weight != 0);
			pheet_assert(node->d < 64);
			for(; init <= node->d; ++init) {
				helper[init] = nullptr;
			}
			Strategy* node_s = comp.deref(node->data);
			if(node_s == nullptr) {
				this->max = node;
				return;
			}
			if(comp(node_s, max_s)) {
				this->max = node;
			}
			Node* tmp = node;
			while(helper[tmp->d] != nullptr) {
			//	pheet_assert(count_nodes(max) == _size);

				if(helper[tmp->d] == this->max ||
						(comp(helper_s[tmp->d], node_s))) {
					pheet_assert(tmp != this->max);
					this->combine_trees(tmp, helper[tmp->d]);
					tmp = helper[tmp->d];
					node_s = helper_s[tmp->d];
				}
				else {
					this->combine_trees(helper[tmp->d], tmp);
				}
			//	pheet_assert(count_nodes(max) == _size);
				helper[tmp->d - 1] = nullptr;
			}
			pheet_assert(helper[tmp->d] == nullptr);
			helper[tmp->d] = tmp;
			helper_s[tmp->d] = node_s;
			for(; init <= tmp->d + 1; ++init) {
				helper[init] = nullptr;
			}
		} while(node != end);
	}


	VolatileStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	Parent* parent;
	bool reconsolidate;
};


template <class Pheet, typename T, class Strategy, class StrategyRetriever>
class VolatileStrategyHeapHeap<Pheet, T, Strategy, Strategy, StrategyRetriever> : public VolatileStrategyHeapBase<Pheet, T> {
public:
	typedef VolatileStrategyHeapHeap<Pheet, T, Strategy, Strategy, StrategyRetriever> Self;
	typedef VolatileStrategyHeapBase<Pheet, T> Base;
	typedef VolatileStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> Comp;
	typedef typename Base::Node Node;

	VolatileStrategyHeapHeap(size_t& total_size, size_t& total_weight, StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:Base(total_size, total_weight), comp(sr), reconsolidate(false) {

	}

	Node* pop() {
		if(reconsolidate) {
			consolidate();
		}
		Node* ret = this->max;

		if(this->max->children != nullptr) {
			this->combine(this->max, this->max->children);
		}

		if(this->max->sub_heap != nullptr) {
			Node* tmp = this->max->sub_heap->pop();
			if(tmp != nullptr) {
				this->combine(this->max, tmp);
			}
		}

		if(this->max->next != this->max) {
			Node* next = this->max->next;
			next->prev = this->max->prev;
			this->max->prev->next = next;
			this->max = next;

			consolidate();
		}
		else {
//			pheet_assert(_size == 0);
//			delete max;
			this->max = nullptr;
		}
		ret->prev = ret;
		ret->next = ret;
		ret->d = 0;
		ret->children = nullptr;
		ret->sub_heap = nullptr;

		return ret;
	}

	void push(Node* node) {
		pheet_assert(node->d == 0);
		pheet_assert(node->children == nullptr);

		Strategy* node_s = comp.deref(node->data);
		if(node_s == nullptr) {
			comp.drop(std::move(node->data));
			--(this->total_size);
			delete node;
			return;
		}
		node->weight = node_s->get_transitive_weight();
		pheet_assert(node->weight);
		this->total_weight += node->weight;

		if(this->max == nullptr) {
			node->next = node;
			node->prev = node;
			this->max = node;
		}
		else {
			node->prev = this->max->prev;
			this->max->prev->next = node;
			this->max->prev = node;
			node->next = this->max;

			Strategy* max_s = comp.deref(this->max->data);
			if(max_s != nullptr) {
	//			pheet_assert(node_s != nullptr);
				if(comp(node_s, max_s)) {
					this->max = node;
				}
			}
		}
	}

	void swap_node(Node* my_node, Node* other_node) {
		if(my_node->next == my_node) {
			other_node->next = other_node;
			other_node->prev = other_node;
		}
		else {
			other_node->next = my_node->next;
			other_node->prev = my_node->prev;
			my_node->next->prev = other_node;
			my_node->prev->next = other_node;
		}
		pheet_assert(other_node->d == 0);
		other_node->children = my_node->children;

		other_node->d = my_node->d;

		Strategy* max_s = comp.deref(this->max->data);
		if(max_s != nullptr) {
			Strategy* node_s = comp.deref(other_node->data);
			if(this->max == my_node) {
				this->max = other_node;
				// Unfortunately this might violate the ordering property, since the ordering might be different for the base strategy
				// so we need to consolidate
				if(comp(max_s, node_s)) {
					reconsolidate = true;
				}
			}
			else if(comp(node_s, max_s)) {
				this->max = other_node;
			}
		}
	}

private:
	void consolidate() {
		reconsolidate = false;
		pheet_assert(this->max != nullptr);

		Strategy* max_s = comp.deref(this->max->data);
		if(max_s == nullptr) {
			return;
		}

		uint8_t init = 0;
		Node* helper[65];
		Strategy* helper_s[65];

		Node* node;
		Node* end = this->max->prev;
		Node* next = this->max;
		do {
			node = next;
			next = node->next;
			pheet_assert(node->weight != 0);
			pheet_assert(node->d < 64);
			for(; init <= node->d; ++init) {
				helper[init] = nullptr;
			}
			Strategy* node_s = comp.deref(node->data);
			if(node_s == nullptr) {
				this->max = node;
				return;
			}
			if(comp(node_s, max_s)) {
				this->max = node;
			}
			Node* tmp = node;
			while(helper[tmp->d] != nullptr) {
			//	pheet_assert(count_nodes(max) == _size);

				if(helper[tmp->d] == this->max ||
						(tmp != this->max &&
								comp(helper_s[tmp->d], node_s))) {
			//		pheet_assert(tmp != this->max);
					this->combine_trees(tmp, helper[tmp->d]);
					node_s = helper_s[tmp->d];
					tmp = helper[tmp->d];
					pheet_assert(node_s != nullptr);
				}
				else {
					this->combine_trees(helper[tmp->d], tmp);
				}
			//	pheet_assert(count_nodes(max) == _size);
				helper[tmp->d - 1] = nullptr;
				helper_s[tmp->d - 1] = nullptr;
			}
			pheet_assert(helper[tmp->d] == nullptr);
			helper[tmp->d] = tmp;
			helper_s[tmp->d] = node_s;
			for(; init <= tmp->d + 1; ++init) {
				helper[init] = nullptr;
			}
		} while(node != end);
	}

	VolatileStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	bool reconsolidate;
};


template <class Pheet, typename TT, class StrategyRetriever>
class VolatileStrategyHeap {
public:
	typedef TT T;
	typedef VolatileStrategyHeapBase<Pheet, T> HeapBase;
	typedef typename HeapBase::Node HeapNode;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef VolatileStrategyHeapPerformanceCounters<Pheet> PerformanceCounters;

	VolatileStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc);
	~VolatileStrategyHeap();

	template <class Strategy>
	void push(T const& item);

	T pop();
	T& peek();

	bool empty() const { return is_empty(); }
	bool is_empty() const;
	size_t size() const { return total_size; }
	size_t transitive_weight() const { return total_weight; }

	static void print_name();

private:
	std::map<std::type_index, HeapBase*> heap_heaps;
	StrategyRetriever sr;
	size_t total_weight;
	VolatileStrategyHeapHeap<Pheet, TT, BaseStrategy, BaseStrategy, StrategyRetriever> root_heap;
	PerformanceCounters pc;

	size_t total_size;
};

template <class Pheet, typename TT, class StrategyRetriever>
VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::VolatileStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc)
:sr(std::move(sr)), total_weight(0), root_heap(total_size, total_weight, sr, heap_heaps), pc(pc), total_size(0) {
	heap_heaps[std::type_index(typeid(BaseStrategy))] = &root_heap;
}

template <class Pheet, typename TT, class StrategyRetriever>
VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::~VolatileStrategyHeap() {
	for(auto i = heap_heaps.begin(); i != heap_heaps.end(); ++i) {
		if(i->second != &root_heap) {
			delete i->second;
		}
	}
}

template <class Pheet, typename TT, class StrategyRetriever>
template <class Strategy>
void VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::push(T const& item) {
	HeapBase*& bheap = heap_heaps[std::type_index(typeid(Strategy))];
	if(bheap == nullptr) {
		bheap = new VolatileStrategyHeapHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>(total_size, total_weight, sr, heap_heaps);
	}
	VolatileStrategyHeapHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>* heap = static_cast<VolatileStrategyHeapHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>*>(bheap);
	HeapNode* node = new HeapNode();
	node->data = item;
	node->d = 0;
	node->children = nullptr;
	node->sub_heap = nullptr;
	heap->push(node);
	++total_size;
}

template <class Pheet, typename TT, class StrategyRetriever>
TT VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::pop() {
//	total_size -= heap->perform_cleanup();
	--total_size;
	HeapNode* node = root_heap.pop();
	total_weight -= node->weight;
	TT ret = node->data;
	delete node;
	return ret;
}

template <class Pheet, typename TT, class StrategyRetriever>
TT& VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::peek() {
	return root_heap.peek()->data;
}

template <class Pheet, typename TT, class StrategyRetriever>
bool VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::is_empty() const {
	pheet_assert((total_size == 0) == root_heap.is_empty());
	pheet_assert((total_weight == 0) == root_heap.is_empty());
	return root_heap.is_empty();
}

template <class Pheet, typename TT, class StrategyRetriever>
void VolatileStrategyHeap<Pheet, TT, StrategyRetriever>::print_name() {
	std::cout << "VolatileStrategyHeap";
}


} /* namespace pheet */
#endif /* VOLATILESTRATEGYHEAP_H_ */
