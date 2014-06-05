
OPTIMIZATION = -O3
#OPTIMIZATION = -O0 -g
CXXFLAGS = $(OPTIMIZATION) -pedantic -Wall -fmessage-length=0 -std=c++11
CXX = g++

LIB_PATH =
INCLUDE_PATH = -I.

LIBS = -lpthread -lhwloc
# Needed for lupiv test
#LIBS =	$(LIBS)  -lblas -llapack

# Flags for Intel Xeon Phi
CXXFLAGS_MIC = $(OPTIMIZATION) -Wall -fmessage-length=0 -std=c++11
LIB_PATH_MIC = 
INCLUDE_PATH_MIC = -I.

