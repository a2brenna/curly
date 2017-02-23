INCLUDE_DIR=$(shell echo ~)/local/include
LIBRARY_DIR=$(shell echo ~)/local/lib
DESDTIR=/
PREFIX=/usr

CXX=g++
CXXFLAGS=-L${LIBRARY_DIR} -I${INCLUDE_DIR} -march=native -O2 -flto -std=c++11 -fPIC -Wall -Wextra -fopenmp -fno-omit-frame-pointer -g

all: benchmark

install: all

benchmark: benchmark.o curly.o
	${CXX} ${CXXFLAGS} -o benchmark benchmark.o curly.o -lboost_program_options -lcurl -ljsoncpp

libcurly.so: curly.o
	${CXX} ${CXXFLAGS} -shared -Wl,-soname,libcurly.so -o libcurly.so curly.o

libcurly.a: curly.o
	ar rcs libcurly.a curly.o

benchmark.o: src/benchmark.cc
	${CXX} ${CXXFLAGS} -c src/benchmark.cc -o benchmark.o

curly.o: src/curly.cc
	${CXX} ${CXXFLAGS} -c src/curly.cc -o curly.o

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
	rm -rf benchmark
