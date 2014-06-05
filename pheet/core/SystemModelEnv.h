/*
 * SystemModelEnv.h
 *
 *  Created on: Jan 31, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef SYSTEMMODELENV_H_
#define SYSTEMMODELENV_H_

#include "../settings.h"

#include "../models/MachineModel/HWLoc/HWLocMachineModel.h"

namespace pheet {

template <class Env, template <class E> class MachineModelT>
class SystemModelEnv {
public:
	typedef MachineModelT<Env> MachineModel;

	template <template <class P> class NewMM>
	using WithMachineModel = SystemModelEnv<Env, NewMM>;

	template <class P>
	using T = SystemModelEnv<P, MachineModelT>;

	template <class P>
	using BT = SystemModelEnv<P, MachineModelT>;
};

template<class Pheet>
using SystemModel = SystemModelEnv<Pheet, HWLocMachineModel>;

}

#endif /* SYSTEMMODELENV_H_ */
