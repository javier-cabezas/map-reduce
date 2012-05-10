ifeq ($(DEBUG),1)
	CXXFLAGS=-std=c++11 -O0 -g -Iinclude -fopenmp
else
	CXXFLAGS=-std=c++11 -O3 -Iinclude -fopenmp
endif
# -ftree-vectorizer-verbose=2
LDFLAGS=-fopenmp
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

