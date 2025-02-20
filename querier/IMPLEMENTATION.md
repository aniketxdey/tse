# CS50 TSE Querier
## Aniket Dey - CS50 - Fall 2024

### Querier

This is the Implementation Spec for the Querier module, which is Lab 6 and part 3 of the Tiny Search Engine. The querier is a program that reads the index file produced by the indexer and page files produced by the crawler, and processes search queries from standard input, returning the results in ranked order.

### Implementation

The querier consists of three main components:

1. `querier.c` - Main program for processing queries
2. `querytest.c` - Testing program for validating query processing
3. `hashtable.c` - Implementation of the hash table data structure

### Data Structures

The querier uses:
* Hash table for storing the index
* Counters structure for tracking word occurrences in documents
* Helper structures for query processing:
  * `unionHelperData` for OR operations
  * `intersectHelperData` for AND operations
  * `maxScoreData` for tracking highest-scoring documents

### Control Flow

**querier**:
1. Parse command-line arguments for any incorrect/invalid args
2. Validate page directory and index file
3. Build in-memory index from index file
4. Enter query processing loop
   - Read query from stdin
   - Parse and validate query
   - Process query against index
   - Rank and display results
5. Clean up

**querytest**:
1. Load index from source
2. Process test queries
3. Verify results against expected output

### Functions

**querier.c**:
```c
int main(const int argc, char* argv[]);
static void mainLoop(index_t* index, const char* pageDirectory);
static void parseQuery(const char* queryStr, char*** words, int* numWords);
static void grammarQuery(const char** words, int numWords, 
                         bool** andSequences, int* numAndSequences,
                         bool** orSequences, int* numOrSequences);
static void processQuery(index_t* index, const char* pageDirectory,
                         bool* andSequences, int numAndSequences,
                         bool* orSequences, int numOrSequences,
                         max_score_t* maxScores);
static void rankResult(const max_score_t* maxScores, const char* pageDirectory);
```

**hashtable.c**:
```c
hashtable_t* hashtable_new(const int num_slots);
bool hashtable_insert(hashtable_t* table, const char* key, counters_t* value);
counters_t* hashtable_get(hashtable_t* table, const char* key);
bool hashtable_delete(hashtable_t* table, const char* key);
void hashtable_delete_all(hashtable_t* table);
```

### Error Handling

The querier implements comprehensive error handling:
* Parameter validation
* Memory allocation checks
* File operation verification
* Memory cleanup on errors
* Error status return values

### Assumptions

* Index file and page files are valid and correspond to each other

### Important Files

* `Makefile` - Makefile for querier
* `querier.c` - As detailed above
* `querytest.c` - As detailed above
* `hashtable.h` - Interface for hash table, located in /common
* `hashtable.c` - As detailed above, located in /common
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

Switch to querier directory, and run make test to test and produce
```bash
cd querier
make test
```

The testing script checks:
1. Invalid arguments handling for all values
2. Valid and invalid query syntax
3. Correctness of query processing
4. Memory leaks with Valgrind

### Exit Status

* 0: Success
* 1: Invalid arguments or usage
* 2: Query processing error

### Assumptions

* Index file and page files are valid and correspond to each other

### Limitations

* Processes only internal URLs
* Does not handle stemming or advanced query syntax