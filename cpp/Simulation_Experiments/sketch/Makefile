OBJECT = main.o cm.o cu.o hcu.o minheap.o BOBHash32.o utils.o
INCLUDE = ./include
LIBRARY = ./lib
GCC = g++
CPP_FLAGS = -O2 -std=c++11

benchmark : $(OBJECT)
	g++ -o benchmark $(OBJECT) -lboost_program_options $(CPP_FLAGS)
main.o : main.cpp
	g++ -c main.cpp -I$(INCLUDE) $(CPP_FLAGS)
cm.o : cm.cpp cm.h sketch.h
	g++ -c cm.cpp -I$(INCLUDE) $(CPP_FLAGS)
cu.o : cu.cpp cu.h sketch.h
	g++ -c cu.cpp -I$(INCLUDE) $(CPP_FLAGS)
hcu.o : hcu.cpp hcu.h sketch.h
	g++ -c hcu.cpp -I$(INCLUDE) $(CPP_FLAGS)
minheap.o : minheap.cpp minheap.h sketch.h
	g++ -c minheap.cpp -I$(INCLUDE) $(CPP_FLAGS)
BOBHash32.o : BOBHash32.cpp BOBHash32.h
	g++ -c BOBHash32.cpp -I$(INCLUDE) $(CPP_FLAGS)	
utils.o : utils.cpp utils.h
	g++ -c utils.cpp $(CPP_FLAGS)
clean :
	find . -name "*.o"  | xargs rm -f
	rm benchmark
