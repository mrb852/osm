.PHONY: clean run

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SOURCES=main.c stack.c
EXECUTABLE=stackfun

default: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)
