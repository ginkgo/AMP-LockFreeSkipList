/*
 * GraphDual.h
 *
 *  Created on: Jul 1, 2012
 *      Author: cederman
 */

#ifndef GRAPHDUAL_H_
#define GRAPHDUAL_H_

#include "../init.h"
#include "pheet/misc/types.h"
#include <vector>

#include <random>

namespace pheet {

class GNode
{
 public:
  size_t connected[3];
  size_t count;
  bool spawnhint;
  size_t taken;
};



class GraphNode {


public:
	GraphNode* edges[3];
	size_t num_edges;
	volatile bool spawned_hint;
	volatile size_t taken;

	GraphNode()
	{

	}

/*	GraphNode(GraphNode* edges, size_t num_edges):edges(edges),num_edges(num_edges)
	{
		spawned_hint = false;
		taken = 0;
	}*/

	size_t getDegree()
	{
		size_t degree = 0;
		for(size_t i=0;i<num_edges;i++)
			if(!edges[i]->isTaken())
				degree++;
		return degree;
	}

	size_t getExtendedDegree()
	{
	  //	  return getDegree();
		size_t degree = 0;
		for(size_t i=0;i<num_edges;i++)
			if(!edges[i]->isTaken())
				degree+=edges[i]->getDegree();
		return degree;
	}

	size_t getNeighbourTaken()
	{
	  size_t nt = 0;
	  for(size_t i=0;i<num_edges;i++)
	    {
	      if(edges[i]->isTaken())
		nt++;
	    }
	  return nt;
	}
	

	size_t getExtendedNeighbourTaken()
	{
	  size_t nt = 0;
	  for(size_t i=0;i<num_edges;i++)
	    {
	    if(edges[i]->isTaken())
	      nt++;

	      nt+=edges[i]->getNeighbourTaken();
	    }
	  return nt;
	}

	bool take()
	{
		if(taken)
			return false;
		return SIZET_CAS((size_t*)&taken,0,1);
	}

	bool isTaken()
	{
		return taken==1;
	}

};


class GraphDual
{
	std::vector<GraphNode*> nodes;

	GraphDual(GraphDual& g)
	{

	}

	void load_from_file(std::string filename)
	{
		unsigned int size = 0;
		FILE * pFile;
		pFile = fopen ( "model.bin" , "rb" );
		size_t read = fread(&size,1,sizeof(int),pFile);
		if(read != sizeof(int)) {
			fclose (pFile);
			throw -1;
		}
		
		GNode* graphdata = new GNode[size];
		read = fread (graphdata,size , sizeof(GNode) , pFile );
	/*	if(read != size*sizeof(GNode)) {
			fclose (pFile);
			throw -1;
		}*/

		fclose (pFile);

		nodes = std::vector<GraphNode*>(size);
		for(unsigned int i=0;i<size;i++)
		{
			nodes[i] = new GraphNode();
		}
		for(unsigned int i=0;i<size;i++)
		{
			for(size_t q=0;q<graphdata[i].count;q++)
			{
				nodes[i]->edges[q]=nodes[graphdata[i].connected[q]];
				if(graphdata[i].connected[q]>=size)
					printf("err");
			}
			nodes[i]->num_edges = graphdata[i].count;
			nodes[i]->spawned_hint = false;
			nodes[i]->taken = 0;
		}

		delete[] graphdata;

	}


	void generate_uniform(size_t size, size_t seed, float p)
	{
		// TODO not updated

		nodes = std::vector<GraphNode*>(size);

		for(size_t i=0;i<size;i++)
			nodes[i] = new GraphNode();

		std::mt19937 rng;
		rng.seed(seed);
	    std::uniform_real_distribution<float> rnd_f(0.0, 1.0);

		std::vector<GraphNode*>* edges = new std::vector<GraphNode*>[size];
		for(size_t i = 0; i < size; ++i) {
			for(size_t j = i + 1; j < size; ++j) {
				if(rnd_f(rng) < p) {
					edges[i].push_back(nodes[j]);
					edges[j].push_back(nodes[i]);
				}
			}

			nodes[i]->num_edges = edges[i].size();
			nodes[i]->taken = 0;
			nodes[i]->spawned_hint = false;
			if(edges[i].size() > 0) {
	//			nodes[i]->edges = new GraphNode*[edges[i].size()];
				for(size_t j = 0; j < edges[i].size(); ++j) {
					nodes[i]->edges[j] = edges[i][j];
				}
			}
			else {
//				nodes[i]->edges = 0;
			}
		}
		delete[] edges;

	}

public:

	GraphDual(size_t size, size_t seed, float p)
	{
		load_from_file("model.bin");
		//generate_uniform(size,seed,p);
	}

	~GraphDual()
	{
		for(size_t i=0;i<nodes.size();i++)
			delete nodes[i];
	}

	size_t size()
	{
		return nodes.size();
	}

	GraphNode* operator[](size_t index)
	{
		return nodes[index];
	}


};


}

#endif /* GRAPHDUAL_H_ */
