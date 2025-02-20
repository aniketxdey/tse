# Makefile for 'common' module
# @author: Aniket Dey

OBJS = pagedir.o index.o word.o
LIB = common.a
L = ../libcs50

# Uncomment the following to turn on verbose memory logging
#TESTING=-DMEMTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I$(L)
CC = gcc
MAKE = make

.PHONY: all clean

all: $(LIB)

# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

# Pattern rule for building object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
pagedir.o: pagedir.h $(L)/webpage.h $(L)/mem.h $(L)/file.h
index.o: index.h $(L)/hashtable.h $(L)/counters.h $(L)/file.h
word.o: word.h

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f $(LIB) *~ *.o
	rm -f core