/*
* TriStripTests.h
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/


#ifndef TriStripTESTS_H_
#define TriStripTESTS_H_

#include "../Test.h"

#include "../init.h"

namespace pheet {

	class TriStripTests
	{
	public:
		void run_test();
	private:
		template <class Pheet, template <class> class Test, bool Prio>
		void test();
	};

}

#endif /* TriStripTESTS_H_ */
