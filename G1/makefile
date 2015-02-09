.PHONY: clean run

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SOURCES=main.c heap.c
BIN=./bin/
EXECUTABLE=$(BIN)heapfun

default: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(BIN)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) -DUNIT_TEST

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)

$(BIN):
	mkdir bin
