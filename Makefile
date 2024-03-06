PROGRAM=main
CC=gcc
CXX=g++
CFLAGS=-Wall

VERBOSE:=

SRC=$(wildcard src/*.cpp)

default: ${PROGRAM}

${PROGRAM}:
	${CXX} ${CFLAGS} -o $@ ${SRC}

test: ${SRC}
	${verbose}${CXX} -DTEST ${CFLAGS} -g -o $@ ${SRC} 

clean: 
	rm -f ${PROGRAM} test
