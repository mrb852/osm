.PHONY: clean run

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SOURCES=main.c heap.c
EXECUTABLE=heapfun

default: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) -DUNIT_TEST

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)
