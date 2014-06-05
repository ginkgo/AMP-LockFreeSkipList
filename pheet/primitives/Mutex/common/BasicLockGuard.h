/*
 * BasicLockGuard.h
 *
 *  Created on: May 29, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef BASICLOCKGUARD_H_
#define BASICLOCKGUARD_H_

namespace pheet {

template <class Pheet, class Mutex>
class BasicLockGuard {
public:
	BasicLockGuard(Mutex& m)
	:m(m) {
		m.lock();
	}
	~BasicLockGuard() {
		m.unlock();
	}

private:
	Mutex& m;
};

} /* namespace pheet */
#endif /* BASICLOCKGUARD_H_ */
