/*
 * MergeStrategyHeapHeap.h
 *
 *  Created on: Mar 8, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef MERGESTRATEGYHEAPHEAP_H_
#define MERGESTRATEGYHEAPHEAP_H_

#include <map>
#include <typeindex>
#include <vector>

namespace pheet {

template <class Pheet, typename T>
class MergeStrategyHeapBase {
public:
	virtual ~MergeStrategyHeapBase() {}

//	virtual bool is_empty() = 0;
	T& peek() {
		return top;
	}
	virtual T pop(/*size_t& total_size*/) = 0;

	size_t parent_index = 0;
	T top;
};

template <class Pheet, class T, class Strategy, class StrategyRetriever>
class MergeStrategyHeapComparator {
public:
//	typedef typename Pheet::Scheduler::TaskDesc TaskDesc;
//	typedef TaskDesc<Strategy> Prep;

	MergeStrategyHeapComparator(StrategyRetriever& sr)
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

template <class Pheet, typename T, class BaseStrategy, class Strategy, class StrategyRetriever>
class MergeStrategyHeapHeap : public MergeStrategyHeapBase<Pheet, T> {
public:
	typedef MergeStrategyHeapBase<Pheet, T> Item;
	typedef MergeStrategyHeapBase<Pheet, T> Base;
	typedef MergeStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> Comp;
//	typedef typename Comp::Prep Prep;

	MergeStrategyHeapHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>& heap_heaps)
	:comp(sr) {
		Base*& base = heap_heaps[std::type_index(typeid(typename Strategy::BaseStrategy))];
		if(base == nullptr) {
			base = new MergeStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>(sr, heap_heaps);
		}
		parent = static_cast<MergeStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>*>(base);
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

	MergeStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	std::vector<Item*> heap;
	MergeStrategyHeapHeap<Pheet, T, BaseStrategy, typename Strategy::BaseStrategy, StrategyRetriever>* parent;
};


template <class Pheet, typename T, class Strategy, class StrategyRetriever>
class MergeStrategyHeapHeap<Pheet, T, Strategy, Strategy, StrategyRetriever> : public MergeStrategyHeapBase<Pheet, T> {
public:
	typedef MergeStrategyHeapBase<Pheet, T> Item;
	typedef MergeStrategyHeapBase<Pheet, T> Base;

	MergeStrategyHeapHeap(StrategyRetriever& sr, std::map<std::type_index, Base*>&)
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

	MergeStrategyHeapComparator<Pheet, T, Strategy, StrategyRetriever> comp;
	std::vector<Item*> heap;
};

} /* namespace pheet */
#endif /* MERGESTRATEGYHEAPHEAP_H_ */
