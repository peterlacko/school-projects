# jednoduche makefile

# makefile
# 21.3.2011
# Autor: Peter Lacko, FIT

all:prvocisla prvocisla-inline

CC=gcc
CFLAGS=-std=c99 -O2 -Wall -pedantic -g

prvocisla: prvocisla.o error.o
	$(CC) $(GCC) -o prvocisla prvocisla.o error.o

prvocisla-inline: prvocisla-inline.o error.o
	$(CC) $(GCC) -DUSE_INLINE -o prvocisla-inline prvocisla-inline.o error.o

prvocisla.o: prvocisla.c error.h
	$(CC) $(CFLAGS) -c prvocisla.c

prvocisla-inline.o: prvocisla.c error.h
	$(CC) $(CFLAGS) -DUSE_INLINE -o prvocisla-inline.o -c prvocisla.c 

error.o: error.c error.h
	$(CC) $(CFLAGS) -c error.c 

clean:
	rm *.o prvocisla prvocisla-inline
