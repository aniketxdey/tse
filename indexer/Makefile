# Makefile for 'indexer' module
# @author: Aniket Dey

# Paths
C = ../common
L = ../libcs50

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$(L) -I$(C)
LIBS = 

LLIBS = $(C)/common.a $(L)/libcs50-given.a

.PHONY: all test clean

all: indexer indextest

indexer: indexer.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

indextest: indextest.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

indexer.o: indexer.c $(L)/mem.h $(C)/pagedir.h $(C)/word.h $(C)/index.h
	$(CC) $(CFLAGS) -c $< -o $@

indextest.o: indextest.c $(C)/index.h
	$(CC) $(CFLAGS) -c $< -o $@

test: indexer indextest
	bash -v testing.sh >& testing.out

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f indexer indextest
	rm -f core