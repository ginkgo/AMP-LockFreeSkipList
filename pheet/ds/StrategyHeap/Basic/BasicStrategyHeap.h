/*
 * BasicStrategyHeap.h
 *
 *  Created on: Mar 27, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICSTRATEGYHEAP_H_
#define BASICSTRATEGYHEAP_H_

#include <map>
#include <typeindex>
#include <vector>
#include <ctime>

#include "BasicStrategyHeapPerformanceCounters.h"

namespace pheet {

template <class Pheet, typename T>
class BasicStrategyHeapBase {
public:
	BasicStrategyHeapBase(): parent_index() {}
	virtual ~BasicStrategyHeapBase() {}

//	virtual bool is_empty() = 0;
	T& peek() {
		return top;
	}
	virtual T pop(/*size_t& total_size*/) = 0;

	size_t parent_index;
	T top;
};

template <class Pheet, class T, class Strategy, class StrategyRetriever>
class BasicStrategyHeapComparator {
public:
//	typedef typename Pheet::Scheduler::TaskDesc TaskDesc;
//	typedef TaskDesc<Strategy> Prep;

	BasicStrategyHeapComparator(StrategyRetriever& sr)
	:sr(sr) {

	}

	Strategy* deref(T& t) {
		return reinterpret_cast<Strategy*>(sr(t));
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
	}
*/
private:
	StrategyRetriever& sr;
/*	TaskDesc<Strategy> td1;
	TaskDesc<Strategy> td2;*/
};

/*
template <class Pheet, typename T>
class BasicStrategyHeapHeapBase : public BasicStrategyHeapBase<Pheet, T> {
public:
	typedef BasicStrategyHeapBase<Pheet, T> Item;

	virtual void reorder_index(size_t index) = 0;
	virtual void pop_index(size_t index) = 0;

	virtual void push(Item* item) = 0;
};*/

template <class Pheet, typename T, class BaseStrategy, class Strategy, class StrategyRetriever>
class BasicStrategyHeapHeap : public BasicStrategyHeapBase<Pheet, T> {
public:
	typedef BasicStrategyHeapBase<Pheet, T> Item;
	typedef BasicStrategyHeapBase<Pheet, T> Base;
	typedef BasicStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> Comp;
//	typedef typename Comp::Prep Prep;

	BasicStrategyHeapHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:comp(sr) {
		Base*& base = heap_heaps[std::type_index(typeid(typename Strategy::BaseStrategy))];
		if(base == nullptr) {
			base = new BasicStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>(sr, heap_heaps);
		}
		parent = static_cast<BasicStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>*>(base);
	}

	bool is_empty() const {
		return heap.empty();
	}
/*
	T& peek() {
		return heap[0]->peek();
	}*/

	T pop(/*size_t& total_size*/) {
		pheet_assert(!heap.empty());
		pheet_assert(heap.size() == 1 || !comp(heap[0]->peek(), heap[1]->peek()));
		return heap[0]->pop(/*total_size*/);
	}

	void push(Item* item) {
		item->parent_index = heap.size();
		heap.push_back(item);
		if(heap.size() == 1) {
			this->top = item->peek();
			parent->push(this);
		}
		else if(bubble_up(heap.size() - 1) == 0) {
			this->top = heap[0]->peek();
			parent->reorder_index(this->parent_index);
		}
	}

	void reorder_index(size_t index) {
		size_t ni = bubble_up(index);
		if(ni == index) {
			bubble_down(index);
		}
		if(ni == 0) {
			this->top = heap[0]->peek();
			parent->reorder_index(this->parent_index);
		}
	}

	void pop_index(size_t index) {
		pheet_assert(index < heap.size());
		if(index == heap.size() - 1) {
			heap.pop_back();
			if(index == 0) {
				parent->pop_index(this->parent_index);
			}
		}
		else {
			heap[index] = heap.back();
			heap[index]->parent_index = index;
			heap.pop_back();
			size_t ni = bubble_up(index);
			if(ni == index) {
				bubble_down(index);
			}
			if(ni == 0) {
				this->top = heap[0]->peek();
				parent->reorder_index(this->parent_index);
			}
		}
	}

	bool verify_parent_index(size_t index) {
		pheet_assert(index < heap.size());
		return heap[index]->parent_index == index;
	}

private:
	void bubble_down(size_t index) {
		size_t next = (index << 1) + 1;
		while(next < heap.size()) {
			size_t nnext = next + 1;
			if(nnext < heap.size() && comp(heap[next]->peek(), heap[nnext]->peek())) {
				next = nnext;
			}
			if(!comp(heap[index]->peek(), heap[next]->peek())) {
				break;
			}
			std::swap(heap[index], heap[next]);
			heap[index]->parent_index = index;
			heap[next]->parent_index = next;
			index = next;
			next = (index << 1) + 1;
		}
	}

	size_t bubble_up(size_t index) {
		size_t next = (index - 1) >> 1;
		while(index > 0 && comp(heap[next]->peek(), heap[index]->peek())) {
			std::swap(heap[next], heap[index]);
			heap[index]->parent_index = index;
			heap[next]->parent_index = next;
			index = next;
			next = (index - 1) >> 1;
		}
		return index;
	}

	BasicStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	std::vector<Item*> heap;
	BasicStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>* parent;
};


template <class Pheet, typename T, class Strategy, class StrategyRetriever>
class BasicStrategyHeapHeap<Pheet, T, Strategy, Strategy, StrategyRetriever> : public BasicStrategyHeapBase<Pheet, T> {
public:
	typedef BasicStrategyHeapBase<Pheet, T> Item;
	typedef BasicStrategyHeapBase<Pheet, T> Base;

	BasicStrategyHeapHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>&)
	:comp(sr) {

	}

	bool is_empty() const {
		return heap.empty();
	}

	T& peek() {
		return heap[0]->peek();
	}

	T pop(/*size_t& total_size*/) {
		pheet_assert(!heap.empty());
		pheet_assert(heap.size() == 1 || !comp(heap[0]->peek(), heap[1]->peek()));
		return heap[0]->pop(/*total_size*/);
	}

	void push(Item* item) {
		item->parent_index = heap.size();
		heap.push_back(item);
		bubble_up(heap.size() - 1);
	}

	void reorder_index(size_t index) {
		size_t ni = bubble_up(index);
		if(ni == index) {
			bubble_down(index);
		}
	}

	void pop_index(size_t index) {
		pheet_assert(index < heap.size());
		if(index == heap.size() - 1) {
			heap.pop_back();
		}
		else {
			heap[index] = heap.back();
			heap[index]->parent_index = index;
			heap.pop_back();
			size_t ni = bubble_up(index);
			if(ni == index) {
				bubble_down(index);
			}
		}
	}

	bool verify_parent_index(size_t index) {
		pheet_assert(index < heap.size());
		return heap[index]->parent_index == index;
	}

private:
	void bubble_down(size_t index) {
		size_t next = (index << 1) + 1;
		while(next < heap.size()) {
			size_t nnext = next + 1;
			if(nnext < heap.size() && comp(heap[next]->peek(), heap[nnext]->peek())) {
				next = nnext;
			}
			if(!comp(heap[index]->peek(), heap[next]->peek())) {
				break;
			}
			std::swap(heap[index], heap[next]);
			heap[index]->parent_index = index;
			heap[next]->parent_index = next;
			index = next;
			next = (index << 1) + 1;
		}
	}

	size_t bubble_up(size_t index) {
		size_t next = (index - 1) >> 1;
		while(index > 0 && comp(heap[next]->peek(), heap[index]->peek())) {
			std::swap(heap[next], heap[index]);
			heap[index]->parent_index = index;
			heap[next]->parent_index = next;
			index = next;
			next = (index - 1) >> 1;
		}
		return index;
	}

	BasicStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	std::vector<Item*> heap;
};


template <class Pheet, typename T, class BaseStrategy, class Strategy, class StrategyRetriever>
class BasicStrategyItemHeap : public BasicStrategyHeapBase<Pheet, T> {
public:
	typedef BasicStrategyHeapBase<Pheet, T> Base;

	BasicStrategyItemHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:comp(sr), sr(sr) {
		Base*& base = heap_heaps[std::type_index(typeid(Strategy))];
		if(base == nullptr) {
			pheet_assert(std::type_index(typeid(Strategy)) != std::type_index(typeid(BaseStrategy)));
			base = new BasicStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>(sr, heap_heaps);
		}
		parent = static_cast<BasicStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>*>(base);
	}

	bool is_empty() const {
		return heap.empty();
	}
/*
	T& peek() {
		return heap[0];
	}*/

	T pop(/*size_t& total_size*/) {
		T ret;
		pheet_assert(heap.size() > 0);
		pheet_assert(!comp(this->top, heap[0]) && !comp(heap[0], this->top));
	//	clean_heap(total_size);
		if(heap.size() == 1) {
			ret = this->top;
			heap.pop_back();
			parent->pop_index(this->parent_index);
		}
		else if(!sr.template is_active<Strategy>(heap.back())) {
			ret = heap.back();
			heap.pop_back();
		}
		else {
			ret = this->top;
			heap[0] = std::move(heap.back());
			heap.pop_back();
			bubble_down(0);
			this->top = heap[0];
			parent->reorder_index(this->parent_index);
			pheet_assert(parent->verify_parent_index(this->parent_index));
		}
		return ret;
	}

	void push(T const& item) {
		heap.push_back(item);
		if(heap.size() == 1) {
			this->top = item;
			parent->push(this);
		}
		else if(bubble_up(heap.size() - 1) == 0) {
			this->top = heap[0];
			parent->reorder_index(this->parent_index);
		}
	}

/*
	size_t perform_cleanup() {
		size_t cleaned = 0;
		// Never cleanup first element as this requires additional cleanup
		while(heap.size() > 1) {
			if(!comp.try_remove(heap.back()))
				return cleaned;
			heap.pop_back();
			++cleaned;
		}
		return cleaned;
	}
*/
private:
	void bubble_down(size_t index) {
		size_t next = (index << 1) + 1;
		while(next < heap.size()) {
			size_t nnext = next + 1;
			if(nnext < heap.size() && comp(heap[next], heap[nnext])) {
				next = nnext;
			}
			if(!comp(heap[index], heap[next])) {
				break;
			}
			std::swap(heap[index], heap[next]);
			index = next;
			next = (index << 1) + 1;
		}
	}

	size_t bubble_up(size_t index) {
		size_t next = (index - 1) >> 1;
		while(index > 0 && comp(heap[next], heap[index])) {
			std::swap(heap[next], heap[index]);
			index = next;
			next = (index - 1) >> 1;
		}
		return index;
	}

	void clean_heap(size_t& total_size) {
		while(heap.size() > 1) {
			if(!sr.clean_item(heap.back())) {
				break;
			}
			--total_size;
			heap.pop_back();
		}
	}

	BasicStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	StrategyRetriever& sr;
	std::vector<T> heap;
	BasicStrategyHeapHeap<Pheet, T, BaseStrategy, Strategy, StrategyRetriever>* parent;
};

template <class Pheet, typename TT, class StrategyRetriever>
class BasicStrategyHeap {
public:
	typedef TT T;
	typedef BasicStrategyHeapBase<Pheet, T> HeapBase;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef BasicStrategyHeapPerformanceCounters<Pheet> PerformanceCounters;

	BasicStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc);
	~BasicStrategyHeap();

	template <class Strategy>
	void push(T const& item);

	T pop();
	T& peek();

	bool empty() const { return is_empty(); }
	bool is_empty() const;
	size_t size() const { return total_size; }

	static void print_name();

private:
	std::map<std::type_index, HeapBase*> item_heaps;
	std::map<std::type_index, HeapBase*> heap_heaps;
	StrategyRetriever sr;
	BasicStrategyHeapHeap<Pheet, TT, BaseStrategy, BaseStrategy, StrategyRetriever> root_heap;
	PerformanceCounters pc;

	size_t total_size;
};

template <class Pheet, typename TT, class StrategyRetriever>
BasicStrategyHeap<Pheet, TT, StrategyRetriever>::BasicStrategyHeap(StrategyRetriever sr, PerformanceCounters& pc)
:sr(std::move(sr)), root_heap(sr, heap_heaps), pc(pc), total_size(0) {
	heap_heaps[std::type_index(typeid(BaseStrategy))] = &root_heap;
}

template <class Pheet, typename TT, class StrategyRetriever>
BasicStrategyHeap<Pheet, TT, StrategyRetriever>::~BasicStrategyHeap() {
	for(auto i = item_heaps.begin(); i != item_heaps.end(); ++i) {
		delete i->second;
	}
	for(auto i = heap_heaps.begin(); i != heap_heaps.end(); ++i) {
		if(i->second != &root_heap) {
			delete i->second;
		}
	}
}

template <class Pheet, typename TT, class StrategyRetriever>
template <class Strategy>
void BasicStrategyHeap<Pheet, TT, StrategyRetriever>::push(T const& item) {
	HeapBase*& bheap = item_heaps[std::type_index(typeid(Strategy))];
	if(bheap == nullptr) {
		bheap = new BasicStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>(sr, heap_heaps);
	}
	BasicStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>* heap = static_cast<BasicStrategyItemHeap<Pheet, TT, BaseStrategy, Strategy, StrategyRetriever>*>(bheap);
//	total_size -= heap->perform_cleanup();
	heap->push(item);
	++total_size;
}

template <class Pheet, typename TT, class StrategyRetriever>
TT BasicStrategyHeap<Pheet, TT, StrategyRetriever>::pop() {
//	total_size -= heap->perform_cleanup();
	--total_size;
	return root_heap.pop(/*total_size*/);
}

template <class Pheet, typename TT, class StrategyRetriever>
TT& BasicStrategyHeap<Pheet, TT, StrategyRetriever>::peek() {
	return root_heap.peek();
}

template <class Pheet, typename TT, class StrategyRetriever>
bool BasicStrategyHeap<Pheet, TT, StrategyRetriever>::is_empty() const {
	pheet_assert((total_size == 0) == root_heap.is_empty());
	return root_heap.is_empty();
}

template <class Pheet, typename TT, class StrategyRetriever>
void BasicStrategyHeap<Pheet, TT, StrategyRetriever>::print_name() {
	std::cout << "BasicStrategyHeap";
}

}

#endif /* BASICSTRATEGYHEAP_H_ */
