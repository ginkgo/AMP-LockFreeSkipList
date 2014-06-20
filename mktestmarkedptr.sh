#!/bin/sh

g++ --std=c++11 -o testmarkedptr testmarkedptr.cpp -lpthread -lhwloc -I. -O3
#g++ --std=c++11 -o testmarkedptr testmarkedptr.cpp -lpthread -lhwloc -I. -O0 -ggdb
