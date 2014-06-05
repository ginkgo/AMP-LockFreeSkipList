/*
 * ReferenceQuicksortLoop.h
 *
 *  Created on: 16.10.2012
 *   Author(s): Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef REFERENCEQUICKSORTLOOP_H_
#define REFERENCEQUICKSORTLOOP_H_

#include <functional>
#include <algorithm>

/*
 *
 */
namespace pheet {

template <class Pheet>
class ReferenceQuicksortLoop {
public:
	ReferenceQuicksortLoop(unsigned int* data, size_t length);
	~ReferenceQuicksortLoop();

	void operator()();

	static const char name[];
private:
	void sort(unsigned int* data, size_t length);

	unsigned int* data;
	size_t length;
};

template <class Pheet>
const char ReferenceQuicksortLoop<Pheet>::name[] = "ReferenceQuicksortLoop";

template <class Pheet>
ReferenceQuicksortLoop<Pheet>::ReferenceQuicksortLoop(unsigned int* data, size_t length)
: data(data), length(length) {

}

template <class Pheet>
ReferenceQuicksortLoop<Pheet>::~ReferenceQuicksortLoop() {

}

template <class Pheet>
void ReferenceQuicksortLoop<Pheet>::operator()() {
	sort(data, length);
}

template <class Pheet>
void ReferenceQuicksortLoop<Pheet>::sort(unsigned int* data, size_t length) {
	struct ToProcess {
		ToProcess(unsigned int* data, size_t length)
		:data(data), length(length) {}

		unsigned int* data;
		size_t length;
	};
	std::vector<ToProcess> tp;

	tp.push_back(ToProcess(data, length));

	while(!tp.empty()) {
		ToProcess t = tp.back();
		tp.pop_back();

		if(t.length <= 1)
			continue;

		unsigned int * middle = std::partition(t.data, t.data + t.length - 1,
				std::bind2nd(std::less<unsigned int>(), *(t.data + t.length - 1)));
		size_t pivot = middle - t.data;
		std::swap(*(t.data + t.length - 1), *middle);    // move pivot to middle

		if(pivot > (t.length - pivot - 1)) {
			tp.push_back(ToProcess(t.data, pivot));
			tp.push_back(ToProcess(t.data + pivot + 1, t.length - pivot - 1));
		}
		else {
			tp.push_back(ToProcess(t.data + pivot + 1, t.length - pivot - 1));
			tp.push_back(ToProcess(t.data, pivot));
		}
	}
}

}

#endif /* REFERENCEQUICKSORTLOOP_H_ */
