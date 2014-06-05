OPTIMIZATION = -O0 -g
CXXFLAGS = $(OPTIMIZATION) -Wall -fmessage-length=0 -std=c++11
CXX = g++-4.7

LIB_PATH =
INCLUDE_PATH = -I../../

LIBS = -lpthread -lhwloc
	
all: sequential parallel

clean:
	rm sequential parallel
	
%:	%.cpp
	$(CXX) $@.cpp -o $@ $(CXXFLAGS) $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)
