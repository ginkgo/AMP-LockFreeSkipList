/*
* InARow.h
*
*  Created on: 22.09.2011
*      Author: Daniel Cederman
*     License: Boost Software License 1.0 (BSL1.0)
*/

#ifndef INAROWGAMETASK_H_
#define INAROWGAMETASK_H_

#include <pheet/pheet.h>
#include "../../../pheet/misc/types.h"
#include "../../../pheet/misc/atomics.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>
#include "InARowTask.h"

using namespace std;

namespace pheet {

	template <class Pheet>
	class InARowGameTask : public Pheet::Task
	{
		unsigned int boardWidth;
		unsigned int boardHeight;
		unsigned int rowLength;
		unsigned int lookAhead;
		unsigned int* scenario;
		bool playScenario;
		char* currBoard;

		
		
	public:
		InARowGameTask(unsigned int width, unsigned int height, unsigned int rowlength, unsigned int lookahead, unsigned int* scenario)
			: boardWidth(width), boardHeight(height), rowLength(rowlength), lookAhead(lookahead), scenario(scenario),playScenario(scenario!=0)
		{
			currBoard = new char[boardHeight*boardWidth];
			memset(currBoard,Empty,boardHeight*boardWidth);
		}
		~InARowGameTask() {}

		virtual void operator()()
		{
			unsigned int turn = 0;
			unsigned int scenarioPtr = 0;
			unsigned int gameMoves[100];

			while(true)
			{
				unsigned int playerMove;

				if(!playScenario)
				{
					printBoard(currBoard);

					while(true)
					{
						cout << "Pick a slot (1-" << boardWidth << "): ";
						cin >> playerMove;
						if(playerMove == 0)
							exit(0);

						playerMove--;

						if(playerMove<boardWidth)
						{
							gameMoves[turn++]=playerMove;
							break;
						}
					}
				}
				else
					playerMove = scenario[scenarioPtr++]-1;

				if(scenario[scenarioPtr-1]==0)
				{
					printf("Did not win :/\n");
					return;
				}

				bool won = move(playerMove, Human);

				if(won)
				{
					if(!playScenario)
					{
						cout << "Player won!" << endl;
						cout << "Game: ";
						for(unsigned int i=0;i<turn;i++)
							cout << gameMoves[i]+1 << " ";
						cout << endl;
						printBoard(currBoard);
					}
					return;
				}

				unsigned int computerMove;
				computerMove = findBestMove();

				won = move(computerMove, Computer);

				if(won)
				{
					if(!playScenario)
					{
						cout << "Computer won!" << endl;
						cout << "Game: ";
						for(unsigned int i=0;i<turn;i++)
							cout << gameMoves[i]+1 << " ";
						cout << endl;
						printBoard(currBoard);
					}
					return;
				}
			}
		}

		unsigned int getBoardWidth()
		{
			return boardWidth;
		}

		unsigned int getBoardHeight()
		{
			return boardHeight;
		}

		unsigned int getRowLength()
		{
			return rowLength;
		}

		

		

		unsigned int eval(char* board, unsigned int x, unsigned int y, bool& winner) 
		{
			// TODO was unsigned and had a negative value. Check why
			int combos[4][2] = {{1,0},{0,1},{1,1},{1,-1}};

			unsigned int val = 0;

			for(int i=0;i<4;i++)
			{
				unsigned int tval = connected(board,x,y,combos[i][0],combos[i][1])+connected(board,x,y,combos[i][0]*-1,combos[i][1]*-1);
				if(tval==getRowLength()+1)
				{
				//	cout << "Winner!";
					winner = true;
				}
				val = tval*tval;
			}
			return val;
		}

		unsigned int connected(char* board,  int x,  int y,  int modx,  int mody)
		{
			unsigned int ctr = 0;
			pheet_assert(y < getBoardHeight());
			pheet_assert(x < getBoardWidth());
			char type = board[y*getBoardWidth()+x];

			while(x<getBoardWidth() && y<getBoardHeight() && x>=0 && y>=0)
			{
				if(board[y*getBoardWidth()+x]==type)
					ctr++;
				else
					break;
				x+=modx;
				y+=mody;
			}
			return ctr;
		}

	private:
		void printBoard(char* board)
		{
			for(unsigned int x=1;x<boardWidth+1;x++)
				cout << x << "";
			cout << endl;
			for(int y=(int)boardHeight-1;y>=0;y--)
			{		
				for(unsigned int x=0;x<boardWidth;x++)
				{
					if(board[y*boardWidth+x]==Empty)
						cout << " ";
					if(board[y*boardWidth+x]==Computer)
						cout << "X";
					if(board[y*boardWidth+x]==Human)
						cout << "O";
				}
				cout << endl;
			}

		}


		bool move(unsigned int slot, Player player)
		{
			unsigned int pos = boardHeight;
			for(unsigned int i=0;i<boardHeight;i++)
			{
				if(currBoard[i*boardWidth+slot]==0)
				{
					pos = i;
					currBoard[i*boardWidth+slot] = player;
					break;
				}
			}
			pheet_assert(pos < boardHeight);
			bool win=false;
			eval(currBoard,slot,pos,win);
			return win;
		}

		unsigned int findBestMove() //typename Task::TEC& tec)
		{
			int* vals = new int[boardWidth];
			memset(vals,0,boardWidth*sizeof(int));

			{
				typename Pheet::Finish f;
				for(unsigned int i=0;i<boardWidth;i++)
					if(currBoard[(getBoardHeight()-1)*getBoardWidth()+i]==0)
						Pheet::template spawn<InARowTask<Pheet> >(lookAhead, currBoard, i, false, &vals[i], this, 0, i==0);
			}

			unsigned int choice = 0;
			int cval = vals[0];

			for(unsigned int i=0;i<boardWidth;i++)
				if(vals[i]<cval)
				{
					choice = i;
					cval = vals[i];
				}
			

/*			cout << "Points: ";
			for(unsigned int i=0;i<boardWidth;i++)
				if(i==choice)
					cout << "[" << vals[i] << "]" << " ";
				else
					cout << vals[i] << " ";
			cout << endl;*/

			delete vals;

			return choice;
		}


	};

}
#endif /* INAROWGAMETASK_H_ */
