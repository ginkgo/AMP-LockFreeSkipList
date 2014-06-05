/*
 * bitops_x86.h
 *
 *  Created on: 28.03.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BITOPS_X86_H_
#define BITOPS_X86_H_

//namespace pheet {

/* find last bit set */
static inline unsigned int find_last_bit_set(unsigned int x) {
if(x == 0) {
	return 0;
}
unsigned int r;

__asm__ ("bsrl %0, %1\n"
         "incl %1\n"
         : "=r"(r)
         : "0"(x));
/*
__asm__("bsrl %1,%0\n\t"
"jnz 1f\n\t"
"movl $-1,%0\n"
"1:" : "=r" (r) : "g" (x));
*/
return r;
}

//}

#endif /* BITOPS_X86_H_ */
