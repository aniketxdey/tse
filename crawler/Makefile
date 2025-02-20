# Makefile for 'crawler' module
# @author: Aniket Dey

# Uncomment the following line to enable verbose memory logging
# TESTING=-DMEMTEST

.PHONY: all test valgrind clean

# Paths
C = ../common
L = ../libcs50

# Compiler/linker settings
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$(L) -I$(C) $(TESTING)
LLIBS = $(L)/libcs50-given.a $(C)/common.a
LIBS =

# Valgrind for memory leak detection
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

# Target rules
all: crawler

crawler: crawler.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

crawler.o: crawler.c $(L)/webpage.h $(C)/pagedir.h $(L)/mem.h $(L)/bag.h $(L)/hashtable.h
	$(CC) $(CFLAGS) -c crawler.c -o crawler.o

test: crawler
	bash -v testing.sh >& testing.out

valgrind: crawler
	mkdir -p ../data/toscrape-1
	$(VALGRIND) ./crawler http://cs50tse.cs.dartmouth.edu/tse/toscrape/ ../data/toscrape-1 1

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f crawler
	rm -f core
