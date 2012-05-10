CXXFLAGS=-std=c++11 -g -O3 -Iinclude -fopenmp
NVCC=nvcc
BINARIES=test
all: $(BINARIES)

test: test.o
	$(CXX) $^ -o $@ -g -fopenmp

main_cuda: main_cuda.o
	g++ $^ -o $@ -g -fopenmp -lcudart

%.o: %.cu
	$(NVCC) -c $^ -o $@ -g -arch sm_20 -Xcompiler -fopenmp -I.

clean:
	rm -f *.o $(BINARIES)

