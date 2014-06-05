/*
 * PriorityQueueMultiSetWrapper.h
 *
 *  Created on: May 30, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef PRIORITYQUEUEMULTISETWRAPPER_H_
#define PRIORITYQUEUEMULTISETWRAPPER_H_

namespace pheet {

template <class Pheet, typename TT, class Compare, template <class, typename, class> class SetT>
class PriorityQueueMultiSetWrapper {
public:
	typedef SetT<Pheet, TT, Compare> Set;

	PriorityQueueMultiSetWrapper() {}
	~PriorityQueueMultiSetWrapper() {}

	inline void push(TT const& item) {
		data.put(item);
	}

	inline TT peek() {
		return data.peek_max();
	}

	inline TT pop() {
		return data.pop_max();
	}

	inline size_t get_length() const {
		return data.get_length();
	}

	inline size_t size() const {
		return get_length();
	}

	inline bool is_empty() const {
		return data.is_empty();
	}

	static void print_name() {
		Set::print_name();
	}

private:
	Set data;
};

} /* namespace pheet */
#endif /* PRIORITYQUEUEMULTISETWRAPPER_H_ */
