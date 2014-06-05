/*
* SORRun.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef SORRUN_H_
#define SORRUN_H_

#include <pheet/pheet.h>
#include "SORTasks.h"
#include <random>
#include <iostream>
#include <fstream>
#include "SORPerformanceCounters.h"
#include "SORAndUTSRun.h"

namespace pheet {

template <class Pheet>
class SORRun {
public:
  SORRun(procs_t cpus, int M, int N, int slices, double omega, int iterations, bool prio, bool runuts, bool runsor, bool usestrat);
	~SORRun();

	void run();

	void print_results();
	void print_headers();

	static void print_name();
	static void print_scheduler_name();

	static char const name[];

	double getTotal();

private:
	unsigned int cpus;
	int iterations;
	SORParams sp;
	double* backend;
	typename Pheet::Environment::PerformanceCounters pc;
	SORPerformanceCounters<Pheet> ppc;
	bool runuts;
	bool runsor;
	bool usestrat;
};


template <class Scheduler>
char const SORRun<Scheduler>::name[] = "SORPartitionMatrix";

template <class Pheet>
double SORRun<Pheet>::getTotal()
{
	return sp.total;
}

template <class Pheet>
  SORRun<Pheet>::SORRun(procs_t cpus, int M, int N, int slices, double omega, int iterations, bool prio, bool runuts, bool runsor, bool usestrat):
cpus(cpus), iterations(iterations),runuts(runuts),runsor(runsor),usestrat(usestrat) {

	sp.M=M;
	sp.N=N;
	sp.slices=slices;
	sp.omega=omega;
	sp.prio = prio;

	ppc.slices_rescheduled_at_same_place.add(slices);

	std::mt19937 rng(42);
	std::uniform_real_distribution<> uni(0,1);

	sp.G = new double*[M];
	backend = new double[M*N];
	for(int i=0;i<M;i++)
		sp.G[i]=&backend[N*i];

	for (int i=0; i<M; i++)
		for (int j=0; j<N; j++)
		{
			sp.G[i][j] = uni(rng) * 1e-6;
		}

/*
	//Compare with reference random values
	ifstream file ("bin/ref/randvals.bin", ios::in|ios::binary);
	file.read((char*)backend,N*M*sizeof(double));
	file.close();
	std::cout << sp.G[621][154] << endl;
*/

}

template <class Pheet>
SORRun<Pheet>::~SORRun() {
	delete[] sp.G;
	delete[] backend;

}

template <class Pheet>
void SORRun<Pheet>::run() {
  typename Pheet::Environment env(cpus,pc);
  //#ifndef SORANDUTS
  //	Pheet::template finish<SORStartTask<Pheet> >(sp, iterations,ppc);
  //#else
  Pheet::template finish<SORAndUTSTask<Pheet> >(sp, iterations,ppc, runuts, runsor, usestrat);
	//#endif

}

template <class Pheet>
void SORRun<Pheet>::print_results() {
	pc.print_values();
	ppc.print_values();
}

template <class Pheet>
void SORRun<Pheet>::print_headers() {
	Pheet::Environment::PerformanceCounters::print_headers();
	ppc.print_headers();
}

template <class Pheet>
void SORRun<Pheet>::print_name() {
	std::cout << name;
}

template <class Pheet>
void SORRun<Pheet>::print_scheduler_name() {
	Pheet::Environment::print_name();
}
}

#endif /* SORRUN_H_ */
