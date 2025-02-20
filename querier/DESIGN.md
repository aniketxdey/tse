# CS50 TSE Querier
## Aniket Dey - CS50 - Fall 2024

## Design Spec

According to the [Indexer Requirements Spec](REQUIREMENTS.md), the TSE *querier* is a program that reads the index file produced by the *indexer* and page files produced by the *crawler*, and processes search queries from standard input, returning the results in ranked order.

### User interface

The querier's only interface with the user is on the command-line and through stdin/stdout. It must always have two command-line arguments:

```
./querier pageDirectory indexFilename
```

For example:

```bash
$ ./querier ../data/letters ../data/letters.index
```

### Inputs and outputs

**Input**: 
1. Command line: pageDirectory and indexFilename
2. Index file: produced by the indexer
3. Page files: in pageDirectory, produced by crawler 
4. Search queries: from stdin, one per line

**Output**: For each query, either:
- "No documents match" if no matches found
- A list of matching documents in decreasing rank order, showing score, document ID, and URL

### Functional decomposition into modules

We implement the following modules:

1. *main*: parses arguments and initializes other modules
2. *mainLoop*: handles the query processing loop
3. *indexBuilder*: builds in-memory index from indexFile
4. *parseQuery*: parses and validates a query string
5. *grammarQuery*: breaks query into sequences based on AND/OR operators
6. *processQuery*: processes the query against the index 
7. *rankResult*: ranks and displays matching documents

We use the following helper modules:

1. *hashtable*: for storing the index
2. *counters*: for tracking word occurrences
3. *webpage*: for loading page files
4. *pagedir*: for validating/reading page directory

### Major data structures

1. *hashtable*: maps words to their occurrence counters
2. *counters*: tracks document IDs and occurrence counts
3. Helper structures:
   - *unionHelperData*: for OR operations
   - *intersectHelperData*: for AND operations
   - *maxScoreData*: for tracking highest-scoring documents

### Processing logic

The querier's main logic:

```
parse command-line arguments
validate pageDirectory and indexFilename
build index from file
while (reading queries from stdin)
    parse and validate query
    process query against index
    rank and display results
cleanup and exit
```

Query logic:

```
split query into words
validate syntax
organize into AND sequences
combine sequences with OR 
compute scores
rank results
display matches
```

### Error handling

The querier gracefully handles:

1. Invalid command-line arguments
2. Invalid/unreadable files
3. Invalid query syntax
4. Memory allocation failures
5. Various edge cases in queries

### Testing plan

*Test Suite*:

1. Valid syntax testing:
   - Single words
   - Multiple words (implicit AND) 
   - Explicit AND/OR operators
2. Invalid Testing
   - Invalid syntax (operators at start/end)
   - Invalid characters
   - Adjacent operators
3. Memory testing:
   - Valgrind checks for memory leaks
