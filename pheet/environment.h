/*
 * environment.h
 *
 *  Created on: Feb 13, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PHEET_ENVIRONMENT_H_
#define PHEET_ENVIRONMENT_H_

#ifdef __linux
#define ENV_LINUX 1
#elif __CYGWIN__
#define ENV_CYGWIN 1
#elif _WIN32
#define ENV_WINDOWS 1
#elif __sun
#define ENV_SOLARIS
#endif

#ifdef __i386
#define ENV_X86		1
#elif __x86_64
#define ENV_X86		1
#elif __amd64
#define ENV_X86		1
#elif __sparc
#define ENV_SPARC	1
#endif

#ifdef __clang__

#if __clang_major__ < 3
#error "Pheet requires clang++ version 3 or higher"
#endif

#define ENV_CLANG 1

#elif __GNUG__

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#error "Pheet requires g++ version 4.7 or higher"
#endif

#define ENV_GCC 1

#else

#define ENV_GCC 1

#endif

#endif /* PHEET_ENVIRONMENT_H_ */
