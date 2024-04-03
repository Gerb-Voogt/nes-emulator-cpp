PROGRAM=main
CC=gcc
CXX=g++
CFLAGS=-Wall

OUTPUT_FOLDER=build

VERBOSE:=

SRC=$(wildcard src/*.cpp)
TST=$(wildcard test/*.cpp)

default: ${PROGRAM}

all:
	make ${PROGRAM}
	make cpu-test

${PROGRAM}:
	mkdir -p ${OUTPUT_FOLDER}
	${CXX} ${CFLAGS} -o ${OUTPUT_FOLDER}/$@ ${SRC}

cpu-test: ${SRC}
	mkdir -p ${OUTPUT_FOLDER}
	${verbose}${CXX} -DTEST ${CFLAGS} -g -o ${OUTPUT_FOLDER}/$@ ${SRC} ${TST}

.PHONY: clean
clean:
	rm -f ${OUTPUT_FOLDER}/${PROGRAM} ${OUTPUT_FOLDER}/cpu-test
	rmdir ${OUTPUT_FOLDER}
