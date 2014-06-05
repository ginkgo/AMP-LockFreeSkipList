/*
 * InARowGame.h
 *
 *  Created on: Nov 23, 2011
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef INAROWGAME_H_
#define INAROWGAME_H_

#include <pheet/pheet.h>
#include "InARowGameTask.h"

#include <iostream>

namespace pheet {

template <class Pheet>
class InARowGame {
public:
	InARowGame(procs_t cpus, unsigned int width, unsigned int height, unsigned int rowlength, unsigned int lookahead, unsigned int* scenario);
	~InARowGame();

	void run();

	void print_results();
	void print_headers();

	static void print_name();
	static void print_scheduler_name();

	static char const name[];
private:
	unsigned int cpus;
	unsigned int width;
	unsigned int height;
	unsigned int rowlength;
	unsigned int lookahead;
	unsigned int* scenario;
	typename Pheet::Environment::PerformanceCounters pc;
};

template <class Pheet>
char const InARowGame<Pheet>::name[] = "InARowGame";

template <class Pheet>
InARowGame<Pheet>::InARowGame(procs_t cpus, unsigned int width, unsigned int height, unsigned int rowlength, unsigned int lookahead, unsigned int* scenario)
:cpus(cpus), width(width), height(height), rowlength(rowlength), lookahead(lookahead), scenario(scenario) {

}

template <class Pheet>
InARowGame<Pheet>::~InARowGame() {

}

template <class Pheet>
void InARowGame<Pheet>::run() {
	typename Pheet::Environment env(cpus,pc);
	Pheet::template finish<InARowGameTask<Pheet> >(width, height, rowlength, lookahead, scenario);
}

template <class Pheet>
void InARowGame<Pheet>::print_results() {
	pc.print_values();
	}

template <class Pheet>
void InARowGame<Pheet>::print_headers() {
	Pheet::Environment::PerformanceCounters::print_headers();
}

template <class Pheet>
void InARowGame<Pheet>::print_name() {
	std::cout << name;
}

template <class Pheet>
void InARowGame<Pheet>::print_scheduler_name() {
	Pheet::Environment::print_name();
}
}

#endif /* INAROWGAME_H_ */
