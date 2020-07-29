CXX=g++
CCFLAGS=-c -Wall #-g
LDFLAGS=-lusb

all: cmliberty 

cmliberty: cmliberty.o liberty.o 
	$(CXX) $^ $(LDFLAGS) -o $@

cmliberty.o: cmliberty.cpp   
	$(CXX) $^ $(CCFLAGS) -o $@
	
liberty.o: liberty.cpp   
	$(CXX) $^ $(CCFLAGS) -o $@
	
#

clean:
	rm -f cmliberty.o *~ 2>/dev/null



