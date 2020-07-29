CC=gcc
CXX=g++
CCFLAGS=-c -Wall #-g
LDFLAGS=-lcomedi -lm

all: cmdelsys

# $@:$^

cmdelsys: cmdelsys.o 
	$(CXX) $^ $(LDFLAGS) -o $@

cmdelsys.o: cmdelsys.cpp   
	$(CXX) $^ $(CCFLAGS) -o $@
	
clean:
	rm -f *~ 2>/dev/null
	rm -f cmcomedi.o
