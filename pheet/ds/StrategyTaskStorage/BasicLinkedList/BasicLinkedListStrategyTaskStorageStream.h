/*
 * BasicLinkedListStrategyTaskStorageStream.h
 *
 *  Created on: Apr 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BASICLINKEDLISTSTRATEGYTASKSTORAGESTREAM_H_
#define BASICLINKEDLISTSTRATEGYTASKSTORAGESTREAM_H_

namespace pheet {

template <class Pheet, class StreamRef>
class BasicLinkedListStrategyTaskStorageStreamRefStrategyRetriever {
public:
	typename StreamRef::BaseStrategy* operator()(StreamRef const& sr) {
		return sr.get_strategy();
	}
/*
	void drop_item(StreamRef&& sr) {

	}*/
};

template <class Pheet, class DataBlock>
class BasicLinkedListStrategyTaskStorageStreamRef {
public:
	typedef BasicLinkedListStrategyTaskStorageStreamRef<Pheet, DataBlock> Self;
	typedef typename DataBlock::Item Item;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef BasicLinkedListStrategyTaskStorageStreamRefStrategyRetriever<Pheet, Self> StrategyRetriever;

	BasicLinkedListStrategyTaskStorageStreamRef() {
	}

	BasicLinkedListStrategyTaskStorageStreamRef(DataBlock* block, int index)
	:block(block), index(index), taken_offset(block->get_taken_offset()) {}

	bool take(Item& item) {
		return block->take(index, taken_offset, item);
	}
	bool take() {
		return block->take(index, taken_offset);
	}

	BaseStrategy* get_strategy() const {
		return block->get_strategy(index, taken_offset);
	}

	DataBlock* block;
	int index;
	size_t taken_offset;
};

template <class Pheet, class TaskStorage, class StealerRef>
class BasicLinkedListStrategyTaskStorageStream {
public:
	typedef BasicLinkedListStrategyTaskStorageStream<Pheet, TaskStorage, StealerRef> Self;
	typedef typename TaskStorage::template ItemT<Self, StealerRef> Item;
//	typedef typename TaskStorage::View View;
	typedef typename TaskStorage::template DataBlockT<Item> DataBlock;
	typedef typename Pheet::Scheduler::BaseStrategy BaseStrategy;
	typedef BasicLinkedListStrategyTaskStorageStreamRef<Pheet, DataBlock> StreamRef;

	BasicLinkedListStrategyTaskStorageStream(TaskStorage& task_storage);
	~BasicLinkedListStrategyTaskStorageStream();

	inline StreamRef get_ref() {
		return StreamRef(last_block, last_index);
	}

	bool has_next() {
		size_t bin = block_index;
		return bin < block->get_size() || (bin == block->get_max_size() && block->get_next() != nullptr && block->get_next()->get_size() > 0);
	}

	void next() {
		pheet_assert(has_next());

		last_block = block;
		last_index = block_index;
		last_block_taken_offset = block->get_taken_offset();

		// Might lead to problems under some relaxed memory consistency models,
		// since a new size may be available even though the value in the array is still in some cache
		// TODO: Fix this with C++11 memory model
		size_t taken_offset = block->get_taken_offset();
		do {
			do {
				++block_index;
			}while(block_index < block->get_size() && block->is_taken(block_index, taken_offset));

			while(block_index == block->get_max_size() || (!block->is_active())) {
				DataBlock* tmp = this->acquire_block(block->get_next());
				block->deregister();
				block = tmp;
				block_index = 0;
			}
		}while(block_index < block->get_size() && block->is_taken(block_index, taken_offset));
		pheet_assert(block_index < block->get_max_size());
		pheet_assert(block != nullptr);
	}

	void stealer_push_ref(StealerRef& stealer) {
		if(!last_block->is_taken(last_index, last_block_taken_offset)) {
			auto sp = last_block->get_data(last_index).stealer_push;
			(this->*sp)(stealer);
		}
	}

	template <class Strategy>
	void stealer_push_ref(StealerRef& stealer) {
		stealer.template push<Strategy>(get_ref());
	}

	bool task_storage_push(TaskStorage& task_storage, StreamRef& stream_ref) {
		auto& data = stream_ref.block->get_data(stream_ref.index);
		auto sp = data.task_storage_push;
		if(stream_ref.take()) {
			(this->*sp)(task_storage, data.strategy, data.item);
			return true;
		}
		return false;
	}

	template <class Strategy>
	void task_storage_push(TaskStorage& task_storage, BaseStrategy* strategy, typename TaskStorage::T const& data) {
		Strategy s(*reinterpret_cast<Strategy*>(strategy));
		s.rebase();
		task_storage.push(std::move(s), data);
	}
private:
	DataBlock* acquire_block(DataBlock* const& block) {
		DataBlock* tmp = block;
		DataBlock* tmp2;
//		typename Pheet::Backoff bo;
		tmp->acquire_block();
		tmp2 = block;
		while(tmp2 != tmp) {
			tmp->deregister();
			tmp = tmp2;
			tmp->acquire_block();
			tmp2 = block;
		}

		return block;
	}

	TaskStorage& task_storage;

	DataBlock* last_block;
	size_t last_index;
	size_t last_block_taken_offset;
	DataBlock* block;
	size_t block_index;
};

template <class Pheet, class TaskStorage, class StealerRef>
BasicLinkedListStrategyTaskStorageStream<Pheet, TaskStorage, StealerRef>::BasicLinkedListStrategyTaskStorageStream(TaskStorage& task_storage)
: task_storage(task_storage),
  block(this->acquire_block(task_storage.get_front_block())),
  block_index(0) {

}

template <class Pheet, class TaskStorage, class StealerRef>
BasicLinkedListStrategyTaskStorageStream<Pheet, TaskStorage, StealerRef>::~BasicLinkedListStrategyTaskStorageStream() {
	// TODO: make sure there are no deadlocks/leaks if we don't deregister!
/*	if(old_view != nullptr)
		old_view->deregister();
	current_view->deregister();*/
}
/*
template <class Pheet, class TaskStorage, class StealerRef>
inline void
BasicLinkedListStrategyTaskStorageStream<Pheet, TaskStorage, StealerRef>::check_current_view() {
	if(task_storage.get_current_view() != current_view) {
		if(old_view == nullptr) {
			old_view = current_view;
		}
		else {
			current_view->deregister();
		}
		current_view = task_storage.acquire_current_view();
	}
	if(old_view != nullptr && block->get_next() == nullptr) {
		old_view->deregister();
		old_view = nullptr;
	}
}
*/
}

#endif /* BASICLINKEDLISTSTRATEGYTASKSTORAGESTREAM_H_ */
