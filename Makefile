PROGRAM=main
CC=gcc
CFLAGS=-Wall -g

VERBOSE:=

SRC=$(wildcard src/*.cpp)
TESTS=$(wildcard test/*.cpp)

all: $(PROGRAM)

$(PROGRAM): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) -lstdc++

test: $(SRC)
	$(verbose)$(CC) -TEST $(CFLAGS) -o $@ $(SRC) $(TEST) -lstdc++

clean: 
	rm -f $(PROGRAM) $(PROGRAM)-test
