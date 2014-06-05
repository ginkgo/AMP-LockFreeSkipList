/*
 * LowDegreeStrategy.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */
#ifndef LowDegreeSTRATEGY_H_
#define LowDegreeSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

  template <class Pheet>
    class RunLastStealFirstStrategy :  public Pheet::Environment::BaseStrategy {
  public:
    typedef RunLastStealFirstStrategy<Pheet> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
  protected:
    bool runlast;
    typename Pheet::Place* localplace;

  public:
  RunLastStealFirstStrategy(bool runlast, size_t weight):runlast(runlast),localplace(Pheet::get_place())
    {
      this->set_transitive_weight(weight);
    }
  RunLastStealFirstStrategy():runlast(false),localplace(Pheet::get_place())
    {
    }
    RunLastStealFirstStrategy(Self& other)
      : BaseStrategy(other),runlast(other.runlast),localplace(other.localplace)
      {
      }

  RunLastStealFirstStrategy(Self&& other)
    : BaseStrategy(other), runlast(other.runlast),localplace(other.localplace)
      {
      }

    ~RunLastStealFirstStrategy() {}

    inline bool prioritize(Self& other) {
      if(runlast != other.runlast)
	{
	  if(runlast && localplace!=Pheet::get_place())
	    return true;
	  return !runlast;
	}


      return BaseStrategy::prioritize(other);
    }

    inline bool forbid_call_conversion() const {
      return true;
    }
  };

  template <class Pheet>
    class LowDegreeStrategy : public  RunLastStealFirstStrategy<Pheet> {
    GraphNode* node;
    size_t degree;
    size_t taken;
  public:

    typedef LowDegreeStrategy<Pheet> Self;
    typedef RunLastStealFirstStrategy<Pheet> BaseStrategy;



  LowDegreeStrategy(/*GraphDual& graph,*/ GraphNode* node, size_t degree,size_t taken):/*graph(graph),*/node(node),degree(degree),taken(taken)
	{
	  BaseStrategy::runlast = false;
	  degree = node->getExtendedDegree();
	  this->set_transitive_weight(1);
	}

	LowDegreeStrategy(Self& other)
	  : BaseStrategy(other), /*graph(other.graph),*/node(other.node),degree(other.degree),taken(other.taken)
	{
	  degree = node->getExtendedDegree();

}

	LowDegreeStrategy(Self&& other)
	  : BaseStrategy(other), /*graph(other.graph),*/node(other.node),degree(other.degree),taken(other.taken)
	{
	  degree = node->getExtendedDegree();
}

	~LowDegreeStrategy() {}

	inline bool prioritize(Self& other) {
	  size_t thisd = degree;// node->getExtendedDegree();
	  size_t otherd = other.degree; //other.node->getExtendedDegree();

	  if(thisd==otherd)
	    return BaseStrategy::prioritize(other);
	  else
	    return thisd < otherd;
	}

	inline bool forbid_call_conversion() const {
	  return false;
        }

};


}

#endif /* LowDegreeSTRATEGY_H_ */
