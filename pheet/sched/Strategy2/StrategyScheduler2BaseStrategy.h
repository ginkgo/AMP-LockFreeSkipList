/*
 * StrategyScheduler2BaseStrategy.h
 *
 *  Created on: 12.08.2013
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef STRATEGYSCHEDULER2BASESTRATEGY_H_
#define STRATEGYSCHEDULER2BASESTRATEGY_H_

#include <limits>

namespace pheet {

template <class Pheet>
class StrategyScheduler2BaseStrategy {
public:
	typedef StrategyScheduler2BaseStrategy<Pheet> Self;
	typedef Self BaseStrategy;
	typedef typename Pheet::Place Place;
	typedef typename Pheet::Scheduler::TaskStorage TaskStorage;

	StrategyScheduler2BaseStrategy()
	{
	}

	StrategyScheduler2BaseStrategy(Self const& other)
	{
	}

	StrategyScheduler2BaseStrategy(Self&& other)
	{
	}

	virtual ~StrategyScheduler2BaseStrategy() {}

	Self& operator=(Self&& other) {
	}

private:
};

}

#endif /* STRATEGYSCHEDULER2BASESTRATEGY_H_ */
