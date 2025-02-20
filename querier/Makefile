# Compiler and Flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I../libcs50 -I../common

# Executable Name
QUERIER_EXEC = querier

# Object Files
OBJ_QUERIER = querier.o ../common/word.o ../libcs50/webpage.o ../libcs50/hashtable.o ../libcs50/counters.o \
              ../common/pagedir.o ../libcs50/file.o ../libcs50/mem.o ../libcs50/set.o ../libcs50/hash.o

# Default target builds the querier executable
all: $(QUERIER_EXEC)

# Linking querier executable
$(QUERIER_EXEC): $(OBJ_QUERIER)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule for compiling .c files to .o files in the common directory
../common/%.o: ../common/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for compiling .c files to .o files in the libcs50 directory
../libcs50/%.o: ../libcs50/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling querier source file into object file
querier.o: querier.c
	$(CC) $(CFLAGS) -c querier.c -o querier.o

# Clean target to remove object files, the executable, and test outputs
.PHONY: clean
clean:
	rm -f $(OBJ_QUERIER) $(QUERIER_EXEC) querier.o
	rm -rf test_output.txt valgrind_output.txt test_queries

# Test target to run the testing script
.PHONY: test
test: $(QUERIER_EXEC) testing.sh
	chmod +x testing.sh
	bash -v testing.sh
