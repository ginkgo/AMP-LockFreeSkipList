/*
 * PrimitivesEnv.h
 *
 *  Created on: Jan 31, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PRIMITIVESENV_H_
#define PRIMITIVESENV_H_

#include "../primitives/Backoff/Exponential/ExponentialBackoff.h"
#include "../primitives/Barrier/Simple/SimpleBarrier.h"
#include "../primitives/Finisher/Basic/Finisher.h"
#include "../primitives/Mutex/BackoffLock/BackoffLock.h"

namespace pheet {

template <class Env, template <class E> class BackoffT, template <class E> class BarrierT, template <class> class FinisherT, template <class> class MutexT>
class PrimitivesEnv {
public:
	template <class P>
	using BT = PrimitivesEnv<P, BackoffT, BarrierT, FinisherT, MutexT>;

	typedef BackoffT<Env> Backoff;
	typedef BarrierT<Env> Barrier;
	typedef FinisherT<Env> Finisher;
	typedef MutexT<Env> Mutex;
	typedef typename Mutex::LockGuard LockGuard;

	template <template <class> class NewMutex>
	using WithMutex = PrimitivesEnv<Env, BackoffT, BarrierT, FinisherT, NewMutex>;

	template <template <class> class NewBackoff>
	using WithBackoff = PrimitivesEnv<Env, NewBackoff, BarrierT, FinisherT, MutexT>;
};

template<class Pheet>
using Primitives = PrimitivesEnv<Pheet, ExponentialBackoff, SimpleBarrier, Finisher, BackoffLock>;

}

#endif /* PRIMITIVESENV_H_ */
