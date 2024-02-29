PROGRAM=main
CC=gcc
CXX=g++
CFLAGS=-Wall -g

VERBOSE:=

SRC=$(wildcard src/*.cpp)

default: ${PROGRAM}

${PROGRAM}:
	${CXX} ${CFLAGS} -o $@ ${SRC}

test: ${SRC}
	${verbose}${CXX} -DTEST ${CFLAGS} -o $@ ${SRC} 

clean: 
	rm -f ${PROGRAM} test
