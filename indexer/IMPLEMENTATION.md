# CS50 TSE Indexer
## Aniket Dey - CS50 - Fall 2024

### Indexer

This is the Implementation Spec for the Indexer module, which is Lab 5 and part 2 of the Tiny Search Engine. The indexer is a standalone program that reads the document files produced by the TSE crawler, builds an index, and writes that index to a file.

### Implementation

The indexer consists of three main components:

1. `indexer.c` - Main program for building index
2. `indextest.c` - Testing program for validating index
3. `index.c` - Implementation of the data structure

### Data Structures

The indexer primarily uses:
* Inverted index
* Word-counters structure containing:
  * Word string
  * Counter set for docID-count pairs

### Control Flow

**indexer**:
1. Parse command-line arguments for any incorrect/invalid args
2. Initialize structure
3. Build index from crawler directory
4. Save index to specified file
5. Clean up

**indextest**:
1. Load index from source 
2. Save index to new file
3. Verify preservation by checking if the .dat files are identical

### Functions

**indexer.c**:
```c
int main(const int argc, char* argv[]);
static bool indexBuild(index_t* index, const char* pageDirectory);
static void indexPage(index_t* index, webpage_t* page, int docID);
```

**index.c**:
```c
index_t* index_new(const int num_slots);
bool index_add(index_t* index, const char* word, const int docID);
bool index_set(index_t* index, const char* word, const int docID, const int count);
counters_t* index_get(index_t* index, const char* word);
bool index_save(index_t* index, const char* filename);
index_t* index_load(const char* filename);
void index_delete(index_t* index)
```
There also helper functions within index.c, written as `save_helper`, `counters_helper`, and `itemdelete`.

### Error Handling

The indexer implements comprehensive error handling:
* Parameter validation
* Memory allocation checks
* File operation verification
* Memory cleanup on errors
* Error status return values

### Assumptions

None

### Important Files

* `Makefile` - Makefile for indexer 
* `indexer.c` - As detailed above
* `indextest.c` - As detailed above
* `index.h` - Interface for index, located in /common 
* `index.c` - As detailed above, located in /common 
* `testing.sh` - Testing script
* `testing.out` - File for testing output

### Compilation

To compile:

In `main` directory, run the below commands to clean previous builds and make all files.
```bash
make clean
make all
```


### Testing

Switch to indexer directory, and run make test to test and produce
```bash
cd indexer
make test
```

The testing script checks:
1. Invalid arguments handling for all values
2. Integration with crawler
3. Memory leaks with Valgrind
4. Index file consistency (indextest)

### Exit Status

* 0: Success
* 1: Invalid arguments or usage

### Assumptions

* None

### Limitations

* Processes only internal URLs