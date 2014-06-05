/*
 * LUPivTests.cpp
 *
 *  Created on: Dec 20, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#include "LUPivTests.h"
#ifdef LUPIV_TEST
#include "Simple/SimpleLUPiv.h"
//#include "LocalityStrategy/LocalityStrategyLUPiv.h"
#include "PPoPPLocalityStrategy/PPoPPLocalityStrategyLUPiv.h"
#endif


#include <pheet/pheet.h>
#include <pheet/sched/Basic/BasicScheduler.h>
#include <pheet/sched/Finisher/FinisherScheduler.h>
#include <pheet/sched/Strategy/StrategyScheduler.h>
#include <pheet/models/MachineModel/HWLoc/HWLocSMTMachineModel.h>

#include <iostream>

namespace pheet {

LUPivTests::LUPivTests() {

}

LUPivTests::~LUPivTests() {

}

void LUPivTests::run_test() {
#ifdef LUPIV_TEST
	std::cout << "----" << std::endl;

       	//this->run_kernel<	Pheet,
	//					LocalityStrategyLUPiv>();
	//this->run_kernel<	Pheet::WithMachineModel<HWLocSMTMachineModel>,
	//					LocalityStrategyLUPiv>();
	//this->run_kernel<	Pheet,
	//					SimpleLUPiv>();
	// this->run_kernel<	Pheet::WithScheduler<BasicScheduler>,
	//					SimpleLUPiv>();
  //this->run_kernel<       Pheet::WithScheduler<BasicScheduler>,
	//			SimpleLUPiv>();

	//	this->run_kernel<	Pheet::WithScheduler<FinisherScheduler>,
	//SimpleLUPiv>();
	//	if(usesstrategy)
	//this->run_kernel<	Pheet::WithScheduler<StrategyScheduler>,
	//					PPoPPLocalityStrategyLUPiv >();

	//	this->run_kernel<	Pheet::WithScheduler<StrategyScheduler>,
	//					PPoPPLocalityStrategyLUPivNoS2C >();
	//else
	//this->run_kernel<	Pheet::WithScheduler<BasicScheduler>,
	//					PPoPPLocalityStrategyLUPiv>();

	this->run_kernel<	Pheet::WithScheduler<StrategyScheduler>,
						SimpleLUPiv >();

	this->run_kernel<	Pheet::WithScheduler<BasicScheduler>,
						SimpleLUPiv>();
#endif
}

}
