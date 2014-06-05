/*
* InARow.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef TriStripTASK_H_
#define TriStripTASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/types.h"
#include "../../../pheet/misc/atomics.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>
#include <queue>
#include <random>

#include "../GraphDual.h"
#include "TriStripPerformanceCounters.h"
#include "LowDegreeStrategy.h"
#include "TriStripResult.h"

using namespace std;

namespace pheet {

	template <class Pheet>
	class TriStripSliceTask;

	template <class Pheet, bool withstrat>
	  class TriStripSpawner;

	template <class Pheet, bool withstrat>
	class TriStripStartTask : public Pheet::Task
	{
		GraphDual& graph;
		TriStripResult<Pheet> result;
		TriStripPerformanceCounters<Pheet> pc;

	public:

		TriStripStartTask(GraphDual& graph, TriStripResult<Pheet>& result, TriStripPerformanceCounters<Pheet>& pc):graph(graph),result(result),pc(pc) { }
		virtual ~TriStripStartTask() {}

		void operator()()
		{

		  std::mt19937 rng;
		  rng.seed(65432);
		  std::uniform_int_distribution<size_t> rnd_st(0, graph.size());

		  size_t startrand =512;
		  for(size_t i = 0; i < startrand; i++)
		    {
		      size_t n = rnd_st(rng);

		      if(!graph[n]->taken && !graph[n]->spawned_hint)
			{
			  graph[n]->spawned_hint = true;
			  if(withstrat)
			    Pheet::template spawn_s<TriStripSliceTask<Pheet>>(LowDegreeStrategy<Pheet>(/*graph,*/graph[n],graph[n]->getExtendedDegree(),graph[n]->getExtendedNeighbourTaken()),/*graph,*/graph[n],result,pc);
			  else
			    Pheet::template spawn<TriStripSliceTask<Pheet>>(graph[n],result,pc);
			}
		    }
		  //typename Pheet::Finish f;

		  //	  return;
		  size_t spawnsetcount = 1024;

		  for(size_t i=0; i<graph.size(); i+=spawnsetcount)
		    {
		      size_t start = i;
		      size_t end = min((i+spawnsetcount),graph.size());

		      if(withstrat)
			Pheet::template spawn_s<TriStripSpawner<Pheet,true>>(RunLastStealFirstStrategy<Pheet>(true,spawnsetcount),&graph,start,end,result,pc);
		      else
			Pheet::template spawn<TriStripSpawner<Pheet,false>>(&graph,start,end,result,pc);

		    }
		  typename Pheet::Finish f;

		}
	};

	template <class Pheet, bool withstrat>
	class TriStripSpawner : public Pheet::Task
	{
		GraphDual* graph;
		size_t start, stop;
		TriStripResult<Pheet> result;
		TriStripPerformanceCounters<Pheet> pc;
	public:

	TriStripSpawner(GraphDual* graph, size_t start, size_t stop, TriStripResult<Pheet>& result, TriStripPerformanceCounters<Pheet>& pc):graph(graph), start(start),stop(stop),result(result),pc(pc) {}
		virtual ~TriStripSpawner() {}

		void operator()()
		{
		  GraphDual& graph = *this->graph;

		  for(size_t i=start; i<stop; i++)
		    {
		      if(!graph[i]->taken && !graph[i]->spawned_hint)
			{
			  graph[i]->spawned_hint = true;

			  if(withstrat)
			    Pheet::template spawn_s<TriStripSliceTask<Pheet>>(LowDegreeStrategy<Pheet>(/*graph,*/graph[i],graph[i]->getExtendedDegree(),graph[i]->getExtendedNeighbourTaken()),/*graph,*/graph[i],result,pc);
			  else
			    Pheet::template spawn<TriStripSliceTask<Pheet>>(graph[i],result,pc);


			}
		    }
		}
	};


	class NodeWithDegree
	{
		GraphNode* n;
		size_t degree;

	public:

		NodeWithDegree(GraphNode* n, size_t degree):n(n),degree(degree)
		{


		}

		GraphNode* getNode() const
		{
			return n;
		}

		bool operator < (const NodeWithDegree& n) const
		{
			return degree > n.degree;
		}

	};

	template <class Pheet>
	class TriStripSliceTask : public Pheet::Task
	{

	//	GraphDual& graph;
		GraphNode* startnode;
		TriStripResult<Pheet> result;
		TriStripPerformanceCounters<Pheet> pc;
	public:

		TriStripSliceTask(/*GraphDual& graph,*/ GraphNode* startnode, TriStripResult<Pheet>& result, TriStripPerformanceCounters<Pheet>& pc):/*graph(graph),*/ startnode(startnode),result(result),pc(pc) {}
		virtual ~TriStripSliceTask() {}

		void operator()()
		{
		  //		  printf("Perform: %d\n",startnode->getExtendedDegree());

			std::vector<GraphNode*> strip;

			std::priority_queue<NodeWithDegree> possiblenextnodes;

			GraphNode* currnode = startnode;
			if(!currnode->take())
				return;

			strip.push_back(currnode);


			for(int i=0;i<2;i++)
			  {
			    currnode=startnode;

			while(true)
			{
			  //		  if(strip.size()>20)
			  // break;
			  //	  printf("Edges %d\n",currnode->num_edges);
				for(size_t d=0;d<currnode->num_edges;d++)
				{
					GraphNode* possnode = currnode->edges[d];

					if(!possnode->isTaken())
					{
						possiblenextnodes.push(NodeWithDegree(possnode,possnode->getExtendedDegree()));
						//	printf("Poss: %d\n",possnode->getExtendedDegree());
					}
				}
				//printf("Size: %d\n",possiblenextnodes.size());

				bool found = false;

				while(!possiblenextnodes.empty())
				{
					GraphNode* g = (possiblenextnodes.top().getNode());
					possiblenextnodes.pop();

					//		printf("Taken: %d\n",g->getExtendedDegree());
					if(g->take())
					{
					//	printf(".");

						found = true;
						currnode = g;
						strip.push_back(g);
						break;
					}

				}

				if(!found)
					break;

				while(!possiblenextnodes.empty())
				{
					GraphNode* n = possiblenextnodes.top().getNode();
					possiblenextnodes.pop();

					if(!(n->spawned_hint || n->isTaken()))
					{
					  //printf(".");
					  //	  	  	n->spawned_hint = true;
									//  Pheet::template spawn_s<TriStripSliceTask<Pheet>>(LowDegreeStrategy<Pheet>(/*graph,*/n,n->getExtendedDegree(),n->getExtendedNeighbourTaken()),/*graph,*/n,result,pc);
					}

				}

			}
			  }
			pc.addstrip(strip);


		}

	};
}
#endif
