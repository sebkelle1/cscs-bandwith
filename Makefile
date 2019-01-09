CXXFLAGS= -O3 -mavx2 -fno-dce
#cc=gcc
CXX=g++
SRC=main.cpp
OBJ=main.o

cscs-bw: ${SRC} kernels-x86_64.cpp timed_run.hpp
	${CXX} ${CXXFLAGS} -c ${SRC}
	${CXX} ${CXXFLAGS} -c kernels-x86_64.cpp
	${CXX} kernels-x86_64.o ${OBJ} -lpthread -o cscs-bw
