/*
 * SORLocalityStrategy.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */
#ifndef SORLOCALITYSTRATEGY_H_
#define SORLOCALITYSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

template <class Pheet>
  class NormalOrHighPrioStrategy :  public Pheet::Environment::BaseStrategy {

    bool highprio;
  public:
    typedef NormalOrHighPrioStrategy<Pheet> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

 NormalOrHighPrioStrategy(bool highprio):highprio(highprio)
    {
      this->set_transitive_weight(1);
    }

    // NormalOrHighPrioStrategy():highprio(true) {this->set_transitive_weight(1000);}

    NormalOrHighPrioStrategy(Self& other)
      : BaseStrategy(other), highprio(other.highprio)
      {}

    NormalOrHighPrioStrategy(Self&& other)
      : BaseStrategy(other), highprio(other.highprio)
      {}

    ~NormalOrHighPrioStrategy() {}

    inline bool forbid_call_conversion() const {
      return false;
    }

    inline bool prioritize(Self& other) {

      if(highprio!=other.highprio)
	return highprio;

      bool res =  BaseStrategy::prioritize(other);

      return res;
    }
  };


  template <class Pheet>
    class UTSStrategy :  public NormalOrHighPrioStrategy<Pheet>
  {
  public:
    typedef UTSStrategy<Pheet> Self;
    typedef NormalOrHighPrioStrategy<Pheet> BaseStrategy;

  UTSStrategy(size_t height, typename Pheet::Place* last_place):BaseStrategy(false),height(height),last_place(last_place)
    {
      //      this->set_transitive_weight(std::min(5000,1<<(std::max((int)1,(int)20-(int)height))));


      // return;
      if(height>4)
	this->set_transitive_weight(1);
      else
	this->set_transitive_weight(10000);
      return;

      this->set_transitive_weight(1);
      return;
      const size_t cost = 1000;
      this->set_transitive_weight((19*19*cost+1)-(height*height*cost));
    }

    UTSStrategy(Self& other)
      : BaseStrategy(other), height(other.height),last_place(Pheet::get_place())
      {}

    UTSStrategy(Self&& other)
      : BaseStrategy(other), height(other.height),last_place(Pheet::get_place())
      {}

    ~UTSStrategy() {}

    inline bool forbid_call_conversion() const {
      return false;
    }

    inline bool prioritize(Self& other) {

      //      return BaseStrategy::prioritize(other);

      procs_t distancetothis = Pheet::get_place()->get_distance(last_place);

      if(distancetothis == 0)
	{
	  // If local task, prioritize recently used
	  return height > other.height;
	}
      else
	{
	  // If stealing, prioritize not recently used
	  return height < other.height;
	}
    }

  private:
    size_t height;
    typename Pheet::Place* last_place;
  };



template <class Pheet>
class SORLocalityStrategy :  public NormalOrHighPrioStrategy<Pheet>
  {
public:

	typedef SORLocalityStrategy<Pheet> Self;
	typedef NormalOrHighPrioStrategy<Pheet> BaseStrategy;

  SORLocalityStrategy(typename Pheet::Place* last_place, ptrdiff_t timestamp):BaseStrategy(true),last_place(last_place),timestamp(timestamp)
	{
		this->set_transitive_weight(1);
	}

	SORLocalityStrategy(Self& other)
	: BaseStrategy(other), last_place(other.last_place),timestamp(other.timestamp)
	{}

	SORLocalityStrategy(Self&& other)
	: BaseStrategy(other), last_place(other.last_place),timestamp(other.timestamp)
	{}

	~SORLocalityStrategy() {}

	inline bool forbid_call_conversion() const {
	  return true;
	}

	inline bool prioritize(Self& other) {

	  procs_t distancetothis = Pheet::get_place()->get_distance(last_place);
	  procs_t distancetoother = Pheet::get_place()->get_distance(other.last_place);

	  if(distancetothis != distancetoother)
	    return distancetothis < distancetoother;

	  //	  return BaseStrategy::prioritize(other);

	  if(distancetothis == 0)
	  {
		// If local task, prioritize recently used
		return timestamp > other.timestamp;
	  }
	  else
	  {
		// If stealing, prioritize not recently used
		return timestamp < other.timestamp;
	  }
	}

private:
	typename Pheet::Place* last_place;
	ptrdiff_t timestamp;
};


}

#endif /* SORLOCALITYSTRATEGY_H_ */
