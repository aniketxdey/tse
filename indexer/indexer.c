/* 
* indexer.c - Module for the Indexer in the TSE. Reads the page directory, builds inverted index, and writes it to a given file.
* @author: Aniket Dey
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../libcs50/webpage.h"
#include "../libcs50/mem.h"
#include "../common/pagedir.h"
#include "../common/index.h"
#include "../common/word.h"

// Function prototypes
bool index_build(index_t* index, const char* pageDirectory);
void index_page(index_t* index, webpage_t* page, int docID);

/*
* main(): Parses arguments, checks validity and initializes other modules.
* Params: number of command-line arguments (argc), array of command-line arguments (argv)
* Returns: 1 if any errors, 0 if successful
*/
int main(int argc, char* argv[]) {
    // Check the argument count and correct usage if incorrect
    if (argc != 3) {
        fprintf(stderr, "Usage: %s pageDirectory indexFilename\n", argv[0]);
        return 1;
    }
    
    char* pageDirectory = argv[1];
    char* indexFilename = argv[2];
    
    // Check if pageDirectory is valid and contains .crawler file
    if (!pagedir_validate(pageDirectory)) {
        fprintf(stderr, "Error: invalid page directory '%s'\n", pageDirectory);
        return 1;
    }
    
    // Verify indexFilename is writable
    FILE* fp = fopen(indexFilename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot write to '%s'\n", indexFilename);
        return 1;
    }
    fclose(fp);
    
    // Initialize index with a starting size of 500
    index_t* index = index_new(500);
    if (index == NULL) {
        fprintf(stderr, "Error: failed to create index (out of memory)\n");
        return 1;
    }
    
    // Build index from page directory files
    if (!index_build(index, pageDirectory)) {
        fprintf(stderr, "Error: failed to build index from '%s'\n", pageDirectory);
        index_delete(index);
        return 1;
    }
    
    // Write completed index to file
    if (!index_save(index, indexFilename)) {
        fprintf(stderr, "Error: failed to save index to '%s'\n", indexFilename);
        index_delete(index);
        return 1;
    }
    
    index_delete(index);
    return 0;
}

/*
* index_build(): Creates index from page directory files
* Params: pointer to index structure (index), directory path containing pages (pageDirectory)
* Returns: true if successful, false on error
*/
bool index_build(index_t* index, const char* pageDirectory) {
    if (index == NULL || pageDirectory == NULL) {
        return false;
    }
    
    int docID = 1;
    webpage_t* page;
    
    // Process each page file until one fails to load
    while ((page = pagedir_load(pageDirectory, docID)) != NULL) {
        index_page(index, page, docID); // Add page's words to index
        webpage_delete(page); // Clean up webpage
        docID++; // Move to next page
    }
    
    return true;
}

/*
* index_page(): Processes single webpage and adds words to index.
* Params: pointer to index (index), webpage to process (page), document ID (docID)
* Returns: void
*/
void index_page(index_t* index, webpage_t* page, int docID) {
    if (index == NULL || page == NULL || docID < 1) {
        return;
    }
    
    int position = 0;
    char* word;
    
    // Get each word, normalize if >=3 chars, add to index
    while ((word = webpage_getNextWord(page, &position)) != NULL) {
        if (strlen(word) >= 3) {
            normalizeWord(word); // Convert to lowercase
            index_add(index, word, docID); // Update word count in index
        }
        free(word); // Clean up word memory
    }
}