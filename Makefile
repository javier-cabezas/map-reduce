CXX=/usr/lib/gcc-snapshot/bin/g++


ifeq ($(DEBUG),1)
	BINARIES=matrixmul stencil test
	CXXFLAGS=-std=c++11 -O0 -g -Iinclude -fopenmp
    LDFLAGS=-fopenmp -g
else
	BINARIES=test
	ifeq ($(NATIVE),1)
		BINARIES+=matrixmul_native stencil_native
	else
		BINARIES+=matrixmul stencil
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

test: test.o
	$(CXX) $^ -o $@ $(LDFLAGS)

%_native.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) -march=native

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -f *.o $(BINARIES)

