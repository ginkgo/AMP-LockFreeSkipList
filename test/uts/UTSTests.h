/*
* UTSTests.h
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/


#ifndef UTSTESTS_H_
#define UTSTESTS_H_

#include "../Test.h"
#include "../init.h"

namespace pheet {

	class UTSTests
	{
	public:
		void run_test();
	private:
		template <class Pheet, template <class> class Test>
		void test();
	};

}

#endif /* INAROWTESTS_H_ */
