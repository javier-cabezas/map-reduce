CXXFLAGS=-std=c++0x -g -O3 -I. -fopenmp
CXX=/usr/lib/gcc-snapshot/bin/g++

all: main main2

main2: main2.o
	$(CXX) $^ -o $@ -g -fopenmp

main: main.o
	$(CXX) $^ -o $@ -g -fopenmp

clean:
	rm -f *.o main main2

