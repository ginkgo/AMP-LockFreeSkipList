/*
 * PlaceBase.h
 *
 *  Created on: May 22, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PLACEBASE_H_
#define PLACEBASE_H_

#include <random>
#include <unordered_map>
#include <typeindex>

namespace pheet {

class PlaceBaseSingletonBase {
public:
	virtual ~PlaceBaseSingletonBase() {}
};

template <class T>
class PlaceBaseSingleton : public PlaceBaseSingletonBase {
public:
	virtual ~PlaceBaseSingleton() {}

	T content;
};

template <class Pheet>
class PlaceBase {
public:
	PlaceBase() {}
	~PlaceBase() {
		for(auto i = singletons.begin(); i != singletons.end(); ++i) {
			pheet_assert(i->second != nullptr);
			delete i->second;
		}
	}

	inline std::mt19937& get_rng() {
		return rng;
	}

	template <class T>
	T& singleton() {
		PlaceBaseSingletonBase*& s = singletons[std::type_index(typeid(T))];
		if(s == nullptr) {
			s = new PlaceBaseSingleton<T>();
		}
		return static_cast<PlaceBaseSingleton<T>*>(s)->content;
	}

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void call(F&& f, TaskParams&& ... params);

private:
	std::mt19937 rng;
	std::unordered_map<std::type_index, PlaceBaseSingletonBase*> singletons;

};


template <class Pheet>
template<class CallTaskType, typename ... TaskParams>
void PlaceBase<Pheet>::call(TaskParams&& ... params) {
	CallTaskType task(std::forward<TaskParams&&>(params) ...);
	task();
}

template <class Pheet>
template<typename F, typename ... TaskParams>
void PlaceBase<Pheet>::call(F&& f, TaskParams&& ... params) {
	f(std::forward<TaskParams&&>(params) ...);
}

} /* namespace pheet */
#endif /* PLACEBASE_H_ */
