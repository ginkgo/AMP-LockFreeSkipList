/*
 * Test.cpp
 *
 *  Created on: 07.04.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#include "Test.h"

namespace pheet {

Test::Test() {
}

Test::~Test() {
}

void Test::check_time(Time & time) {
	time = Timer::now();
}

double Test::calculate_seconds(Time const& start, Time const& end) {
	return 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}
}
