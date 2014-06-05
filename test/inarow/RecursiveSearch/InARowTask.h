/*
* InARow.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef INAROWTASK_H_
#define INAROWTASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/types.h"
#include "../../../pheet/misc/atomics.h"
//#include "../../test_schedulers.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>

using namespace std;

namespace pheet {

	enum Player
	{
		Empty   = 0,
		Computer = 1, // The computer player - the minimizer
		Human = 2  // The human player - the maximizer
	};

	template <class Pheet>
		class DepthStrategy : public Pheet::Environment::BaseStrategy {
		public:
			typedef DepthStrategy<Pheet> Self;
			typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

			DepthStrategy(size_t depth)
			: depth(depth) {
//				this->set_memory_footprint(1);
				this->set_transitive_weight(1000-depth);
			}

			DepthStrategy(Self& other)
			: BaseStrategy(other), depth(other.depth) {}

			DepthStrategy(Self&& other)
			: BaseStrategy(other), depth(other.depth) {}

			~DepthStrategy() {}

			inline bool prioritize(Self& other) {
				if(this->get_place() == other.get_place() && this->get_place() == Pheet::get_place())
					return this->depth > other.depth;
				else
					return this->depth < other.depth;
			}

		private:
			size_t depth;
		};


	template <class Pheet>
	class InARowGameTask;

	template <class Pheet>
	class InARowTask : public Pheet::Task
	{
		unsigned int depth;
		char* board;
		unsigned int move;
		bool player;
		int* val;
		InARowGameTask<Pheet>* iar;
		unsigned int movecount;
		int boardval;
		bool debug;
	public:
		InARowTask(unsigned int depth, char* board, unsigned int move, bool player, int* val, InARowGameTask<Pheet>* iar, int boardval, bool debug)
			:depth(depth),board(board),move(move),player(player),val(val),iar(iar),boardval(boardval),debug(debug)  { }
		virtual ~InARowTask() {}

		void operator()() //typename Task::TEC& tec)
		{
			//if(debug)
				//cout << "Depth: " << depth << " Boardval: "<< boardval << endl;

			char* newboard = new char[iar->getBoardWidth()*iar->getBoardHeight()];
			memcpy(newboard,board,iar->getBoardWidth()*iar->getBoardHeight()*sizeof(char));

	//		bool found = false;

			unsigned int pos = 0;
			for(unsigned int i=0;i<iar->getBoardHeight();i++)
			{
				pheet_assert(move < iar->getBoardWidth());
				if(newboard[i*iar->getBoardWidth()+move]==0)
				{
					pos = i;
					newboard[i*iar->getBoardWidth()+move] = player?Human:Computer;
		//			found = true;
					break;
				}
			}

			bool win = false; 
			unsigned int nval = iar->eval(newboard, move, pos, win);

			boardval += player?nval:-nval;

			if(win)
			{
				*val = player?(0x7fffffff):(0x80000000);
				delete[] newboard;
				return;
			}

			if(depth==0)
			{
			//	if(debug)
			//	iar->printBoard(newboard);
				delete[] newboard;
				*val = boardval+1;
				return;
			}

			int* vals = new int[iar->getBoardWidth()];
			memset(vals,0,iar->getBoardWidth()*sizeof(int));

			{
				typename Pheet::Finish f;

				for(unsigned int i = 0; i<iar->getBoardWidth(); i++)
				{
					if(newboard[(iar->getBoardHeight()-1)*iar->getBoardWidth()+i]!=0)
						continue;

					Pheet::template spawn_s<InARowTask<Pheet> >(DepthStrategy<Pheet>(depth), depth-1, newboard, i, !player, &vals[i],iar, boardval, debug&& i==0);
										//tec.template spawn<InARowTask<Task> >(depth-1, newboard, i, !player, &vals[i],iar, boardval, debug&& i==0);
				}
			}

			/*if(debug)
			{
				if(player)
					cout << "Player: ";
				else
					cout << "Computer: ";
			}
			if(debug)
			for(unsigned int i=0;i<iar->getBoardWidth();i++)
				cout << vals[i] << " ";*/

			*val = player?(0x7fffffff):(0x80000000);

			for(unsigned int i=0;i<iar->getBoardWidth();i++)
				if(vals[i]!=0 && ((vals[i]<*val && player)||(vals[i]>*val && !player)))
				{
//					if(debug)
	//					cout << vals[i] << " ";

					*val = vals[i];
				}

			//if(debug)
				//cout << endl;
			delete[] newboard;
			delete[] vals;
		}

	private:
		
	};
}
#endif /* INAROWTASK_H_ */
