#!/bin/sh

g++ --std=c++11 -o settest settest.cpp -lpthread -lhwloc -I. -O3 -DNDEBUG
#g++ --std=c++11 -o settest settest.cpp -lpthread -lhwloc -I. -O0 -ggdb
