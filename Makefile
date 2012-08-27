CXX=/usr/lib/gcc-snapshot/bin/g++

ifeq ($(DEBUG),1)
	CXXFLAGS=-std=c++11 -O0 -g -Iinclude -fopenmp
    LDFLAGS=-fopenmp -g
else
	CXXFLAGS=-std=c++11 -O3 -g -Iinclude -fopenmp -march=native
    LDFLAGS=-fopenmp
endif

# -ftree-vectorizer-verbose=2
NVCC=nvcc
BINARIES=test
all: $(BINARIES)

test: test.o
	$(CXX) $^ -o $@ $(LDFLAGS)

main_cuda: main_cuda.o
	g++ $^ -o $@ -fopenmp -lcudart

%.o: %.cu
	$(NVCC) -c $^ -o $@ -arch sm_20 -Xcompiler -fopenmp -I.

clean:
	rm -f *.o $(BINARIES)

