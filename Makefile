CXX=/usr/lib/gcc-snapshot/bin/g++


ifeq ($(DEBUG),1)
	BINARIES=matrixmul stencil convolution test
	CXXFLAGS=-std=c++11 -O0 -g -Iinclude -fopenmp -DDEBUG=1
    LDFLAGS=-fopenmp -g
else
	BINARIES=test
	ifeq ($(NATIVE),1)
		BINARIES+=matrixmul_native stencil_native convolution_native
	else
		BINARIES+=matrixmul stencil convolution
	endif
	CXXFLAGS=-std=c++11 -O3 -g -Iinclude -fopenmp
    LDFLAGS=-fopenmp
endif

# -ftree-vectorizer-verbose=2
all: $(BINARIES)

matrixmul: matrixmul.o
	$(CXX) $^ -o $@ $(LDFLAGS)

matrixmul_native: matrixmul_native.o
	$(CXX) $^ -o $@ $(LDFLAGS)

stencil: stencil.o
	$(CXX) $^ -o $@ $(LDFLAGS)

stencil_native: stencil_native.o
	$(CXX) $^ -o $@ $(LDFLAGS)

convolution: convolution.o
	$(CXX) $^ -o $@ $(LDFLAGS)

convolution_native: convolution_native.o
	$(CXX) $^ -o $@ $(LDFLAGS)

test: test.o
	$(CXX) $^ -o $@ $(LDFLAGS)

%_native.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) -march=native

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -f *.o $(BINARIES)

