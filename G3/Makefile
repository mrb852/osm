.PHONY: clean run

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SOURCES=queue.c
EXECUTABLE=queue

default: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)
