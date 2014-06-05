/*
 * FibolikeHeap.h
 *
 *  Created on: Jun 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FIBOLIKEHEAP_H_
#define FIBOLIKEHEAP_H_

#include <pheet/misc/bitops.h>
#include <vector>
#include <utility>

namespace pheet {

template <typename TT>
struct FibolikeHeapField {
	typedef FibolikeHeapField<TT> Self;

	FibolikeHeapField() {}
	FibolikeHeapField(TT&& data, std::vector<Self>&& sub)
	: data(std::move(data)), sub(std::move(sub)) {}

	TT data;
	std::vector<Self> sub;
};

template <class Pheet, typename TT, class Comparator = std::less<TT> >
class FibolikeHeap {
public:
	typedef TT T;
	typedef FibolikeHeapField<TT> Field;

	FibolikeHeap()
	:max_pos(0), fields_filled(0), _size(0) {

	}

	~FibolikeHeap() {

	}

	void push(T item) {
		fields_filled = std::max(combine0(std::move(item)) + 1, fields_filled);
		++_size;
	}

	TT peek() {
		pheet_assert(!empty());
		return fields[max_pos].data;
	}

	TT pop() {
		pheet_assert(!empty());
		T ret = std::move(fields[max_pos].data);
		std::vector<Field> sub = std::move(fields[max_pos].sub);
		pheet_assert(fields[max_pos].sub.empty());

		if(max_pos != 0) {
			_size -= (1 << max_pos);

			int pos = max_pos;
			max_pos = 0;
			int npos = combine0(std::move(sub[0].data));
			++_size;
			for(int i = 1; i < pos; ++i) {
				npos = std::max(combine(i, std::move(sub[i].data), std::move(sub[i].sub)), npos);
				pheet_assert(sub[i].sub.empty());
				_size += 1 << i;
			}

			if(pos == fields_filled - 1 && npos < pos) {
				pheet_assert(npos == pos - 1);
				--fields_filled;
			}
			else {
				for(int i = npos + 1; i < fields_filled; ++i) {
					if((!fields[i].sub.empty()) && is_less(fields[max_pos].data, fields[i].data)) {
						pheet_assert(i != max_pos);
						max_pos = i;
					}
				}
			}
		}
		else if(_size != 1) {
			--_size;

			bool first = true;
			for(int i = 1; i < fields_filled; ++i) {
				if((!fields[i].sub.empty()) && (first || is_less(fields[max_pos].data, fields[i].data))) {
					max_pos = i;
					first = false;
				}
			}
		}
		else {
			_size = 0;
			fields_filled = 0;
		}

		return ret;
	}

	bool empty() const {
		return _size == 0;
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
	int combine0(T item) {
		std::vector<Field> sub;
		int pos = find_first_bit_set(~_size) - 1;

		for(int i = 0; i < pos; ++i) {
			if(is_less(item, fields[i].data)) {
				fields[i].sub.push_back(Field(std::move(item), std::move(sub)));
				pheet_assert(sub.empty());
				item = std::move(fields[i].data);
				sub = std::move(fields[i].sub);
				pheet_assert(fields[i].sub.empty());
			}
			else {
				sub.push_back(Field(std::move(fields[i].data), std::move(fields[i].sub)));
				pheet_assert(fields[i].sub.empty());
			}
		}

		if(max_pos <= pos) {
			max_pos = pos;
		}
		else if(is_less(fields[max_pos].data, item)) {
			max_pos = pos;
		}

		pheet_assert(fields[pos].sub.empty());

		fields[pos].data = std::move(item);
		fields[pos].sub = std::move(sub);

		pheet_assert(sub.empty());

		return pos;
	}

	int combine(int offset, T&& item, std::vector<Field>&& sub) {
		pheet_assert(offset > 0);

		int pos = find_first_bit_set(~(_size >> offset)) + offset - 1;

		for(int i = offset; i < pos; ++i) {
			pheet_assert(!fields[i].sub.empty());

			if(is_less(item, fields[i].data)) {
				fields[i].sub.push_back(Field(std::move(item), std::move(sub)));
				pheet_assert(sub.empty());
				item = std::move(fields[i].data);
				sub = std::move(fields[i].sub);
				pheet_assert(fields[i].sub.empty());
			}
			else {
				sub.push_back(Field(std::move(fields[i].data), std::move(fields[i].sub)));
				pheet_assert(fields[i].sub.empty());
			}
		}

		if(max_pos >= offset && max_pos <= pos) {
			pheet_assert(max_pos != pos);
			max_pos = pos;
		}
		else if(is_less(fields[max_pos].data, item)) {
			max_pos = pos;
		}

		pheet_assert(fields[pos].sub.empty());

		fields[pos].data = std::move(item);
		fields[pos].sub = std::move(sub);

		pheet_assert(sub.empty());

		return pos;
	}

	Field fields[64];
	int max_pos;
	int fields_filled;

	size_t _size;
	Comparator is_less;
};

} /* namespace pheet */
#endif /* FIBOLIKEHEAP_H_ */
