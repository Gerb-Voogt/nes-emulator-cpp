PROGRAM=main
CC=gcc
CXX=g++
CFLAGS=-Wall -g
LIBS=-lncurses

OUTPUT_FOLDER=build

VERBOSE:=

SRC=$(wildcard src/*.cpp)
TST=$(wildcard test/*.cpp)

default: ${PROGRAM}

${PROGRAM}:
	mkdir -p ${OUTPUT_FOLDER}
	${CXX} ${CFLAGS} -o ${OUTPUT_FOLDER}/$@ ${SRC} ${LIBS}

cpu-test: ${SRC}
	mkdir -p ${OUTPUT_FOLDER}
	${verbose}${CXX} -DTEST ${CFLAGS} -g -o ${OUTPUT_FOLDER}/$@ ${SRC} ${TST} ${LIBS}

test-all: ${SRC}
	mkdir -p ${OUTPUT_FOLDER}
	make cpu-test
	./build/cpu-test


.PHONY: clean
clean:
	rm -f ${OUTPUT_FOLDER}/${PROGRAM} ${OUTPUT_FOLDER}/cpu-test
	rmdir ${OUTPUT_FOLDER}
