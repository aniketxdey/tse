/* 
 * querier.c - A program that reads an index file and a page directory, processes queries from stdin, and returns results.
 * @author: Aniket Dey
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include "webpage.h"
#include "hashtable.h"
#include "counters.h"
#include "file.h"
#include "pagedir.h"
#include "word.h"

// Types

// Struct to hold data for union operations
typedef struct {
    counters_t* runningSum;
} unionHelperData_t;

// Struct to hold data for intersection operations
typedef struct {
    counters_t* tempProduct; 
    counters_t* wordCounters; 
} intersectHelperData_t;

// Struct to hold data for ranking results
typedef struct {
    int maxDocID; 
    int maxScore;
} maxScoreData_t;

// Function Prototypes
char* takeQuery(void);
char** parseQuery(char* query);
void printQuery(char** wordArray);
int cleanQuery(char** wordArray);
char*** grammarQuery(char** wordArray);
hashtable_t* indexBuilder(char* indexFilename);
counters_t* wordMatch(char* wordinRankArray, hashtable_t* index);
counters_t* processAndSequence(char** andSequence, hashtable_t* index);
counters_t* processQuery(char*** rankArray, hashtable_t* index);
void rankResult(counters_t* runningSum, const char* pageDirectory);
void mainLoop(const char* pageDirectory, const char* indexFilename);
void parseArgs(const int args, char* argv[], char** pageDirectory, char** indexFilename);

// Helper functions
void unionHelper(void* arg, const int docID, int count);
void intersectHelper(void* arg, const int docID, int count);
void rankHelper(void* arg, const int docID, int count);
void delete_word_counter_pairs(void* arg, const char* key, void* item);
void copy_counter_item(void* arg, const int key, const int count);
void freeWordArray(char** wordArray);
void freeRankArray(char*** rankArray);
void cleanup(hashtable_t* index, char* query, char** wordArray, char*** rankArray);

// Helper Functions

/*
 * freeRankArray(): Frees the rank array memory.
 * Params: array of word arrays (rankArray)
 * Returns: none
 */
void freeRankArray(char*** rankArray) {
    if (rankArray != NULL) {
        for (int i = 0; rankArray[i] != NULL; i++) {

            for (int j = 0; rankArray[i][j] != NULL; j++) {
                free(rankArray[i][j]); // Free each string
            }
            free(rankArray[i]); // Free array of strings
        }
        free(rankArray); 
    }
}

/*
 * cleanup(): Frees allocated memory and cleans up resources.
 * Params: index hashtable (index), query string (query), word array (wordArray), rank array (rankArray)
 * Returns: none
 */
void cleanup(hashtable_t* index, char* query, char** wordArray, char*** rankArray) {
    if (index != NULL) {
        // Iterate through the hashtable and delete word-counter pairs
        hashtable_iterate(index, NULL, delete_word_counter_pairs);
        // Delete the hashtable itself
        hashtable_delete(index, NULL);
    }
    if (query != NULL) {
        free(query); 
    }
    if (wordArray != NULL) {
        freeWordArray(wordArray); 
    }
    if (rankArray != NULL) {
        freeRankArray(rankArray); 
    }
}

/*
 * copy_counter_item(): Copies a counter item.
 * Params: argument (arg), document ID (key), count (count)
 * Returns: none
 */
void copy_counter_item(void* arg, const int key, const int count) {
    counters_t* dest = arg; 
    if (dest != NULL) {
        counters_set(dest, key, count); 
    }
}

/*
 * delete_word_counter_pairs(): Deletes word-counter pairs in hashtable.
 * Params: argument (arg), word key (key), counters item (item)
 * Returns: none
 */
void delete_word_counter_pairs(void* arg, const char* key, void* item) {
    if (key != NULL) {
        free((char*)key); 
    }
    if (item != NULL) {
        counters_delete(item);
    }
}

/*
 * delete_item(): Deletes a counters item.
 * Params: item to delete (item)
 * Returns: none
 */
void delete_item(void* item) {
    if (item != NULL) {
        counters_delete((counters_t*)item);
    }
}

/*
 * freeWordArray(): Frees the word array memory.
 * Params: array of words (wordArray)
 * Returns: none
 */
void freeWordArray(char** wordArray) {
    if (wordArray != NULL) {
        // Iterate through each word in the array
        for (int i = 0; wordArray[i] != NULL; i++) {
            free(wordArray[i]); // Free each word
        }
        free(wordArray);
    }
}

/**************** takeQuery ****************/
/*
 * takeQuery(): Reads user input for the query.
 * Params: none
 * Returns: pointer to the query string
 */
char* takeQuery() {
    // If input is from terminal, prompt the user
    if (isatty(fileno(stdin))) {
        printf("Query? ");
        fflush(stdout); // Ensure prompt is displayed
    }

    char* line = NULL; // Pointer to store the input line
    size_t len = 0; 
    ssize_t read = getline(&line, &len, stdin); // Read the input line
    
    if (read == -1) { 
        free(line); 
        return NULL;
    }

    // Remove trailing newline if present
    if (line[read - 1] == '\n') {
        line[read - 1] = '\0';
    }

    return line; // Return the query string
}

/**************** parseQuery ****************/
/*
 * parseQuery(): Parses the query string into words.
 * Params: query string (query)
 * Returns: array of words
 */
char** parseQuery(char* query) {
    // Check for NULL or empty query
    if (query == NULL || *query == '\0') {
        return NULL;
    }

    // Initialize word array with initial capacity
    int capacity = 10;
    char** wordArray = malloc(capacity * sizeof(char*));
    if (wordArray == NULL) {
        return NULL;
    }
    // Initialize all pointers to NULL
    for (int i = 0; i < capacity; i++) {
        wordArray[i] = NULL;
    }

    int wordCount = 0; // Number of words parsed
    char* c = query; 

    // Process each word in the query
    while (*c != '\0') {
        while (isspace(*c)) c++;
        if (*c == '\0') break; 

        // Verify word characters and count length
        char* wordStart = c; 
        int wordLen = 0; 
        while (*c != '\0' && !isspace(*c)) {
            if (!isalpha(*c)) {
                fprintf(stderr, "Invalid query syntax: bad character '%c' in query.\n", *c);
                // Free previously allocated words
                for (int i = 0; i < wordCount; i++) {
                    free(wordArray[i]);
                }
                free(wordArray);
                return NULL;
            }
            wordLen++;
            c++;
        }

        // Allocate space for the new word
        char* word = malloc(wordLen + 1);
        if (word == NULL) {
            for (int i = 0; i < wordCount; i++) {
                free(wordArray[i]);
            }
            free(wordArray);
            return NULL;
        }

        // Copy and convert word to lowercase
        for (int i = 0; i < wordLen; i++) {
            word[i] = tolower(wordStart[i]);
        }
        word[wordLen] = '\0';

        // Expand array if needed
        if (wordCount >= capacity) {
            int newCapacity = capacity * 2;
            char** newArray = malloc(newCapacity * sizeof(char*));
            if (newArray == NULL) {
                free(word);
                for (int i = 0; i < wordCount; i++) {
                    free(wordArray[i]);
                }
                free(wordArray);
                return NULL;
            }
            // Copy existing pointers to the new array
            for (int i = 0; i < wordCount; i++) {
                newArray[i] = wordArray[i];
            }
            // Initialize new slots to NULL
            for (int i = wordCount; i < newCapacity; i++) {
                newArray[i] = NULL;
            }
            free(wordArray); 
            wordArray = newArray;
            capacity = newCapacity;
        }

        wordArray[wordCount++] = word; 
    }

    if (wordCount == 0) {
        free(wordArray);
        return NULL;
    }
    
    wordArray[wordCount] = NULL; // Null-terminate the array
    return wordArray;
}

/**************** grammarQuery ****************/
/*
 * grammarQuery(): Organizes words into grammatical sequences.
 * Params: array of words (wordArray)
 * Returns: array of arrays of words
 */
char*** grammarQuery(char** wordArray) {
    if (wordArray == NULL) return NULL;

    int capacity = 10; 
    int rankCapacity = 10;      

    // Allocate initial space for rankArray
    char*** rankArray = malloc(rankCapacity * sizeof(char**));
    if (rankArray == NULL) return NULL;

    // Allocate initial space for andSequence
    char** andSequence = malloc(capacity * sizeof(char*));
    if (andSequence == NULL) {
        free(rankArray);
        return NULL;
    }

    int andLength = 0;   
    int rankLength = 0; 

    // Iterate through each word in the wordArray
    for (int i = 0; wordArray[i] != NULL; i++) {
        if (strcmp(wordArray[i], "or") == 0) {
            andSequence[andLength] = NULL;
            // Allocate space for the current sequence in rankArray
            rankArray[rankLength] = malloc((andLength + 1) * sizeof(char*));
            if (rankArray[rankLength] == NULL) {
                free(andSequence);
                freeRankArray(rankArray);
                return NULL;
            }

            // Duplicate strings for this sequence
            for (int j = 0; j < andLength; j++) {
                rankArray[rankLength][j] = strdup(andSequence[j]);
                if (rankArray[rankLength][j] == NULL) {
                    // Cleanup on failure
                    for (int k = 0; k < j; k++) {
                        free(rankArray[rankLength][k]);
                    }
                    free(rankArray[rankLength]);
                    free(andSequence);
                    freeRankArray(rankArray);
                    return NULL;
                }
            }
            rankArray[rankLength][andLength] = NULL; // Null-terminate the sequence
            rankLength++; 
            andLength = 0; 

            // Expand rankArray if needed
            if (rankLength >= rankCapacity) {
                rankCapacity *= 2;
                char*** temp = realloc(rankArray, rankCapacity * sizeof(char**));
                if (temp == NULL) {
                    free(andSequence);
                    freeRankArray(rankArray);
                    return NULL;
                }
                rankArray = temp;
            }
        } else if (strcmp(wordArray[i], "and") == 0) {
            // Skip the "and" keyword as it's implicit in the sequence
            continue;
        } else {  
            // Add word to current andSequence
            andSequence[andLength] = wordArray[i];
            andLength++;

            // Expand andSequence if needed
            if (andLength >= capacity) {
                capacity *= 2;
                char** temp = realloc(andSequence, capacity * sizeof(char*));
                if (temp == NULL) {
                    free(andSequence);
                    freeRankArray(rankArray);
                    return NULL;
                }
                andSequence = temp;
            }
        }
    }

    // Handle the last sequence after the loop
    if (andLength > 0) {
        rankArray[rankLength] = malloc((andLength + 1) * sizeof(char*));
        if (rankArray[rankLength] == NULL) {
            free(andSequence);
            freeRankArray(rankArray);
            return NULL;
        }

        for (int j = 0; j < andLength; j++) {
            rankArray[rankLength][j] = strdup(andSequence[j]);
            if (rankArray[rankLength][j] == NULL) {
                // Cleanup on failure
                for (int k = 0; k < j; k++) {
                    free(rankArray[rankLength][k]);
                }
                free(rankArray[rankLength]);
                free(andSequence);
                freeRankArray(rankArray);
                return NULL;
            }
        }
        rankArray[rankLength][andLength] = NULL; // Null-terminate the sequence
        rankLength++;
    }

    free(andSequence); // Free the temporary andSequence array
    rankArray[rankLength] = NULL;
    return rankArray; 
}

/**************** indexBuilder ****************/
/*
 * indexBuilder(): Builds index from index file.
 * Params: index filename (indexFilename)
 * Returns: pointer to the index hashtable
 */
hashtable_t* indexBuilder(char* indexFilename) {
    if (indexFilename == NULL) return NULL;

    // Create a new hashtable with an initial size
    hashtable_t* index = hashtable_new(200);  
    if (index == NULL) return NULL;

    // Open the index file for reading
    FILE* indexFile = fopen(indexFilename, "r");
    if (indexFile == NULL) {
        hashtable_delete(index, NULL);
        return NULL;
    }

    char* line = NULL;
    size_t len = 0;     
    bool error_occurred = false; 

    // Read each line from the index file
    while (getline(&line, &len, indexFile) != -1 && !error_occurred) {
        char word[200]; 
        int docID, count; 
        
        counters_t* counter = counters_new(); 
        if (counter == NULL) {
            error_occurred = true;
            break;
        }

        char* remaining = line; 
        int charsRead = 0;      
        
        if (sscanf(remaining, "%199s%n", word, &charsRead) != 1) {
            counters_delete(counter); 
            continue; 
        }
        remaining += charsRead; // Move the pointer past the word

        // Parse docIDs and counts from the remaining part of the line
        while (sscanf(remaining, "%d %d%n", &docID, &count, &charsRead) == 2) {
            counters_set(counter, docID, count); 
            remaining += charsRead; 
        }

        char* wordCopy = strdup(word); // Duplicate the word string
        if (wordCopy == NULL) {
            counters_delete(counter); // Delete counters on failure
            error_occurred = true; 
            break;
        }

        // Insert the word and its counters into the hashtable
        if (!hashtable_insert(index, wordCopy, counter)) {
            free(wordCopy); // 
            counters_delete(counter); 
            error_occurred = true; 
            break;
        }
    }

    free(line); 
    fclose(indexFile); 

    if (error_occurred) { 
        hashtable_iterate(index, NULL, delete_word_counter_pairs);
        hashtable_delete(index, delete_item); 
        return NULL; 
    }

    return index; 
}

/**************** unionHelper ****************/
/*
 * unionHelper(): Helper function for union of counters.
 * Params: argument (arg), document ID (docID), count (count)
 * Returns: none
 */
void unionHelper(void* arg, const int docID, int count) {
    unionHelperData_t* data = arg; // Cast the argument to unionHelperData_t
    if (data != NULL && data->runningSum != NULL) {
        int existingCount = counters_get(data->runningSum, docID); 
        counters_set(data->runningSum, docID, existingCount + count); // Update the count
    }
}

/**************** intersectHelper ****************/
/*
 * intersectHelper(): Helper function for intersection of counters.
 * Params: argument (arg), document ID (docID), count (count)
 * Returns: none
 */
void intersectHelper(void* arg, const int docID, int count) {
    intersectHelperData_t* data = arg; // Cast the argument to intersectHelperData_t
    if (data != NULL && data->tempProduct != NULL && data->wordCounters != NULL) {
        int wordCount = counters_get(data->wordCounters, docID); 
        if (wordCount > 0) { 
            int minCount = (count < wordCount) ? count : wordCount; 
            counters_set(data->tempProduct, docID, minCount); 
        }
    }
}

/**************** rankHelper ****************/
/*
 * rankHelper(): Helper function for ranking documents.
 * Params: argument (arg), document ID (docID), count (count)
 * Returns: none
 */
void rankHelper(void* arg, const int docID, int count) {
    maxScoreData_t* data = arg; 
    if (data != NULL && count > data->maxScore) {
        data->maxScore = count;  
        data->maxDocID = docID;  
    }
}

/**************** wordMatch ****************/
/*
 * wordMatch(): Finds matching documents for a word in the index.
 * Params: word to match (wordinRankArray), index hashtable (index)
 * Returns: pointer to counters of matching documents
 */
counters_t* wordMatch(char* wordinRankArray, hashtable_t* index) {
    if (wordinRankArray == NULL || index == NULL) return NULL;

    counters_t* findWord = hashtable_find(index, wordinRankArray); 
    counters_t* wordCounts = counters_new(); 
    if (wordCounts == NULL) return NULL;

    if (findWord != NULL) { 
        counters_iterate(findWord, wordCounts, copy_counter_item); 
    }

    return wordCounts; // Return the counters of matching documents
}

/**************** processAndSequence ****************/
/*
 * processAndSequence(): Processes 'AND' sequence of words against the index.
 * Params: array of words in 'AND' sequence (andSequence), index hashtable (index)
 * Returns: pointer to counters of matching documents
 */
counters_t* processAndSequence(char** andSequence, hashtable_t* index) {
    if (andSequence == NULL || index == NULL) return NULL;

    counters_t* runningProduct = NULL; // Initialize running product of counters

    // Iterate through each word in the 'AND' sequence
    for (int i = 0; andSequence[i] != NULL; i++) {
        counters_t* wordCounters = wordMatch(andSequence[i], index); // Get counters for the word
        if (wordCounters == NULL) {
            counters_delete(runningProduct);
            return NULL;
        }

        if (runningProduct == NULL) { // If first word in the sequence
            runningProduct = counters_new(); 
            if (runningProduct == NULL) {
                counters_delete(wordCounters); // Delete wordCounters on failure
                return NULL;
            }
            counters_iterate(wordCounters, runningProduct, copy_counter_item); // Copy wordCounters to runningProduct
        } else {
            counters_t* tempProduct = counters_new(); // Create a temporary counters for intersection
            if (tempProduct == NULL) {
                counters_delete(wordCounters); 
                counters_delete(runningProduct); 
                return NULL;
            }

            intersectHelperData_t data = {tempProduct, wordCounters}; 
            counters_iterate(runningProduct, &data, intersectHelper); 

            counters_delete(runningProduct); 
            runningProduct = tempProduct; // Update runningProduct to the intersection result
        }
        counters_delete(wordCounters); 
    }

    return runningProduct; 
}

/**************** processQuery ****************/
/*
 * processQuery(): Processes the query against the index.
 * Params: array of 'AND' sequences (rankArray), index hashtable (index)
 * Returns: pointer to counters of matching documents
 */
counters_t* processQuery(char*** rankArray, hashtable_t* index) {
    if (rankArray == NULL || index == NULL) return NULL;

    counters_t* runningSum = counters_new(); // Initialize running sum of counters
    if (runningSum == NULL) return NULL;

    unionHelperData_t data = {runningSum}; // Prepare data for union operations

    // Iterate through each 'AND' sequence in the rankArray
    for (int i = 0; rankArray[i] != NULL; i++) {
        counters_t* andResult = processAndSequence(rankArray[i], index); // Process the 'AND' sequence
        if (andResult == NULL) {
            counters_delete(runningSum); 
            return NULL;
        }

        counters_iterate(andResult, &data, unionHelper); // Union the 'AND' result into runningSum
        counters_delete(andResult); // Delete the 'AND' result as it's no longer needed
    }

    return runningSum; // Return the combined counters of all sequences
}

/**************** rankResult ****************/
/*
 * rankResult(): Ranks and displays search results.
 * Params: counters of matching documents (runningSum), page directory (pageDirectory)
 * Returns: none
 */
void rankResult(counters_t* runningSum, const char* pageDirectory) {
    if (runningSum == NULL || pageDirectory == NULL) return;

    maxScoreData_t data; 
    int resultsFound = 0; 

    while (1) {
        data.maxDocID = -1; 
        data.maxScore = 0; 

        // Iterate to find the document with the highest score
        counters_iterate(runningSum, &data, rankHelper);

        if (data.maxScore == 0) break; 

        resultsFound = 1; // At least one result found
        webpage_t* page = pagedir_load((char*)pageDirectory, data.maxDocID); 

        if (page != NULL) { 
            char* url = webpage_getURL(page); 
            printf("score: %d doc: %d url: %s\n", data.maxScore, data.maxDocID, url); 
            webpage_delete(page);
        }

        // Reset the score for the ranked document to avoid re-ranking
        counters_set(runningSum, data.maxDocID, 0);
    }

    if (!resultsFound) { 
        printf("No documents match.\n");
    }
}

/**************** printQuery ****************/
/*
 * printQuery(): Prints the parsed query words.
 * Params: array of words (wordArray)
 * Returns: none
 */
void printQuery(char** wordArray) {
    if (wordArray == NULL || wordArray[0] == NULL) {
        return; // Nothing to print
    }
    
    // Iterate through each word and print it
    for (int i = 0; wordArray[i] != NULL; i++) {
        printf("%s", wordArray[i]);
        if (wordArray[i + 1] != NULL) { // If not the last word, print a space
            printf(" ");
        }
    }
    printf("\n"); // Newline after printing all words
}

/**************** cleanQuery ****************/
/*
 * cleanQuery(): Cleans the word array by processing each word.
 * Params: array of words (wordArray)
 * Returns: 1 if any errors, 0 if successful
 */
int cleanQuery(char** wordArray) {
    if (wordArray == NULL || wordArray[0] == NULL) {
        fprintf(stderr, "Error: Empty query.\n"); // Report empty query error
        return 1;
    }

    // Check if the first word is an operator
    if ((strcmp(wordArray[0], "and") == 0) || (strcmp(wordArray[0], "or") == 0)) {
        fprintf(stderr, "Error: '%s' cannot be first\n", wordArray[0]); // Report operator at start error
        return 1;
    }

    // Check for adjacent operators in the query
    for (int i = 0; wordArray[i] != NULL; i++) {
        if ((strcmp(wordArray[i], "and") == 0 || strcmp(wordArray[i], "or") == 0)) {
            if (wordArray[i + 1] != NULL && 
                (strcmp(wordArray[i + 1], "and") == 0 || strcmp(wordArray[i + 1], "or") == 0)) {
                fprintf(stderr, "Error: '%s' and '%s' cannot be adjacent\n", 
                        wordArray[i], wordArray[i + 1]); // Report adjacent operators error
                return 1;
            }
        }
    }

    // Check if the last word is an operator
    int last = 0;
    while (wordArray[last] != NULL) {
        last++;
    } 
    last--; 
    
    if (last >= 0 && (strcmp(wordArray[last], "and") == 0 || strcmp(wordArray[last], "or") == 0)) {
        fprintf(stderr, "Error: '%s' cannot be last\n", wordArray[last]); // Report operator at end error
        return 1;
    }

    return 0; // Query is clean
}

/**************** mainLoop ****************/
/*
 * mainLoop(): Runs the main query processing loop.
 * Params: page directory (pageDirectory), index filename (indexFilename)
 * Returns: none
 */
void mainLoop(const char* pageDirectory, const char* indexFilename) {
    if (pageDirectory == NULL || indexFilename == NULL) return; // Validate inputs

    hashtable_t* index = indexBuilder((char*)indexFilename); // Build the index from the index file
    if (index == NULL) return; // Exit if index building failed

    while (1) { // Infinite loop to process queries
        char* query = takeQuery(); 
        if (query == NULL) break; 

        char** wordArray = parseQuery(query); // Parse the query into words
        if (wordArray == NULL) { // If parsing failed
            free(query); 
            continue; 
        }

        // Validate the parsed query
        if (wordArray[0] == NULL || cleanQuery(wordArray) != 0) {
            free(query); // Free the query string
            freeWordArray(wordArray);
            continue; // Continue to the next iteration
        }

        printf("Query: "); // Print the query prefix
        printQuery(wordArray); // Print the actual query words

        char*** rankArray = grammarQuery(wordArray); // Organize words into grammatical sequences
        if (rankArray == NULL) { // If grammar processing failed
            free(query); // Free the query string
            freeWordArray(wordArray); 
            continue; // Continue to the next iteration
        }

        counters_t* results = processQuery(rankArray, index); // Process the query against the index
        if (results != NULL) {
            rankResult(results, pageDirectory); 
            counters_delete(results); // Delete the results counters
        }

        // Cleanup allocated resources for this query
        freeRankArray(rankArray);
        freeWordArray(wordArray); 
        free(query); 
    }

    hashtable_delete(index, delete_item); // Delete the index hashtable
}

/**************** parseArgs ****************/
/*
 * parseArgs(): Parses command-line arguments.
 * Params: number of arguments (args), array of arguments (argv), page directory pointer (pageDirectory), index filename pointer (indexFilename)
 * Returns: none
 */
void parseArgs(const int args, char* argv[], char** pageDirectory, char** indexFilename) {
    // Check for correct number of arguments and non-NULL pointers
    if (args != 3 || argv == NULL || pageDirectory == NULL || indexFilename == NULL) {
        fprintf(stderr, "Usage: ./querier pageDirectory indexFilename\n"); // Print usage message
        exit(1); 
    }

    *pageDirectory = argv[1]; // Assign pageDirectory from arguments
    *indexFilename = argv[2]; // Assign indexFilename from arguments

    // Validate the page directory
    if (!pagedir_validate(*pageDirectory)) {
        fprintf(stderr, "Error: Invalid pageDirectory %s\n", *pageDirectory); // Report invalid directory
        exit(1); 
    }

    // Check if index file exists and is readable
    FILE* indexFile = fopen(*indexFilename, "r");
    if (indexFile == NULL) {
        fprintf(stderr, "%s file does not exist or cannot be read\n", *indexFilename); // Report file error
        exit(1); 
    }
    fclose(indexFile); // Close the file as it exists and is readable
}

/**************** main ****************/
/*
 * main(): Parses arguments, checks validity and initializes other modules.
 * Params: number of command-line arguments (argc), array of command-line arguments (argv)
 * Returns: 1 if any errors, 0 if successful
 */
int main(const int argc, char* argv[]) {
    char* pageDirectory = NULL; 
    char* indexFilename = NULL; 

    parseArgs(argc, argv, &pageDirectory, &indexFilename); // Parse and validate arguments
    mainLoop(pageDirectory, indexFilename); 

    return 0; // Return success
}
