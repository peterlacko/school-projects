# Makefile pro program readerWriter
PROGRAM = readerWriter
PROGRAM_SOURCE = readerWriter.c
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread
CC=gcc

$(PROGRAM):$(PROGRAM_SOURCE)
	$(CC) $(CFLAGS) $(PROGRAM_SOURCE) -o $(PROGRAM)

clean:
	rm readerWriter
