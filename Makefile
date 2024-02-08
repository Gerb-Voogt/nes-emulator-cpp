PROGRAM=main
CC=gcc
CFLAGS=-Wall -g

VERBOSE:=

SRC=$(wildcard src/*.cpp)

all: $(PROGRAM)

$(PROGRAM): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) -lstdc++

test: $(SRC)
	$(verbose)$(CC) -DTEST $(CFLAGS) -o $@ $(SRC) $(TESTS) -lstdc++

clean: 
	rm -f $(PROGRAM) test
