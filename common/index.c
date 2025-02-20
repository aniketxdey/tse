/* 
* index.c - Index module for TSE, and provides the data structure for an inverted index
* @author: Aniket Dey
*/

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"
#include "../libcs50/file.h"
#include "../libcs50/mem.h"

// Local Types
typedef struct word_counters {
    char* word;
    counters_t* ctrs;
} word_counters_t;

typedef struct index {
    hashtable_t* ht; // Hashtable mapping words to word_counters
} index_t;

// Functions
static void itemdelete(void* item);
static void save_helper(void* arg, const char* key, void* item);
static void counters_helper(void* arg, const int key, const int count);

/*
* index_new(): Creates new index with specified hashtable size
* Params: number of slots for hashtable (num_slots)
* Returns: pointer to new index, or null if error
*/
index_t* index_new(const int num_slots) {
    if (num_slots <= 0) {
        return NULL; // If no slots in hashtable, return null
    }
    
    index_t* index = mem_malloc(sizeof(index_t));
    if (index == NULL) {
        return NULL; // If index is null, return null
    }
    
    index->ht = hashtable_new(num_slots);
    if (index->ht == NULL) {
        mem_free(index); // Free the index and return null
        return NULL;
    }
    
    return index;
}

/*
* index_add(): Increments word occurrence for given docID
* Params: index pointer (index), word to add (word), document ID (docID)
* Returns: true if successful, false on error
*/
bool index_add(index_t* index, const char* word, const int docID) {
    if (index == NULL || word == NULL || docID <= 0) {
        return false;
    }
    
    // Look for existing word_counters
    word_counters_t* wc = hashtable_find(index->ht, word);
    if (wc == NULL) {
        // Create new word_counters structure
        wc = mem_malloc(sizeof(word_counters_t));
        if (wc == NULL) {
            return false;
        }
        
        // Create counter set
        wc->ctrs = counters_new();
        if (wc->ctrs == NULL) {
            mem_free(wc);
            return false;
        }
        
        // Copy word; free memory and return false if any errors
        wc->word = mem_malloc(strlen(word) + 1);
        if (wc->word == NULL) {
            counters_delete(wc->ctrs);
            mem_free(wc);
            return false;
        }
        strcpy(wc->word, word);
        
        // Insert the word into hashtable
        if (!hashtable_insert(index->ht, wc->word, wc)) {
            mem_free(wc->word);
            counters_delete(wc->ctrs);
            mem_free(wc);
            return false;
        }
    }
    
    // Increment counter for this docID
    counters_add(wc->ctrs, docID);
    return true;
}

/*
* index_set(): Sets exact count for evey word-docID pair
* Params: index pointer (index), word to set (word), document ID (docID), count to set (count)
* Returns: true if successful, false on error
*/
bool index_set(index_t* index, const char* word, const int docID, const int count) {
    if (index == NULL || word == NULL || docID <= 0 || count <= 0) {
        return false;
    }
    
    word_counters_t* wc = hashtable_find(index->ht, word);
    if (wc == NULL) {
        // Create new word_counters structure
        wc = mem_malloc(sizeof(word_counters_t));
        if (wc == NULL) {
            return false;
        }
        
        // Create counter set
        wc->ctrs = counters_new();
        if (wc->ctrs == NULL) {
            mem_free(wc);
            return false;
        }
        
        // Copy word
        wc->word = mem_malloc(strlen(word) + 1);
        if (wc->word == NULL) {
            counters_delete(wc->ctrs); // If word copy null; delete counters and free memory
            mem_free(wc);
            return false;
        }
        strcpy(wc->word, word);
        
        // Insert into hashtable
        if (!hashtable_insert(index->ht, wc->word, wc)) {
            mem_free(wc->word);
            counters_delete(wc->ctrs);
            mem_free(wc);
            return false;
        }
    }
    
    counters_set(wc->ctrs, docID, count);
    return true;
}

/*
* index_get(): Retrieves counter set for given word.
* Params: index pointer (index), word to look up (word)
* Returns: pointer to counters set, or null if not found
*/
counters_t* index_get(index_t* index, const char* word) {
    if (index == NULL || word == NULL) {
        return NULL;
    }
    word_counters_t* wc = hashtable_find(index->ht, word);
    return (wc != NULL) ? wc->ctrs : NULL;
}

/*
* index_save(): Writes index to file in specified format.
* Params: index pointer (index), filename to write to (filename)
* Returns: true if successful, false on error
*/
bool index_save(index_t* index, const char* filename) {
    if (index == NULL || filename == NULL) {
        return false;
    }
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        return false;
    }

    hashtable_iterate(index->ht, fp, save_helper);
    fclose(fp);
    return true;
}


// Helper function for the save function - writes oneword and its count to the document at a time
static void save_helper(void* arg, const char* key, void* item) {
    FILE* fp = arg;
    if (fp == NULL || key == NULL || item == NULL) {
        return;
    }
    
    word_counters_t* wc = item;
    fprintf(fp, "%s", wc->word);
    
    if (wc->ctrs != NULL) {
        fprintf(fp, " ");
        counters_iterate(wc->ctrs, fp, counters_helper);
    }
    fprintf(fp, "\n");
}

// Helper method for save_helper, writes a single id-count pair to the fail
static void counters_helper(void* arg, const int key, const int count) {
    FILE* fp = arg;
    if (fp != NULL && count > 0) {
        fprintf(fp, "%d %d ", key, count);
    }
}

/*
* index_load(): Reads index from file in format
* Params: filename to read from (filename)
* Returns: pointer to new index, or null if error
*/
index_t* index_load(const char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }

    int num_lines = file_numLines(fp);
    if (num_lines == 0) {
        fclose(fp);
        return NULL;
    }
    
    index_t* index = index_new(num_lines);
    if (index == NULL) {
        fclose(fp);
        return NULL;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        if (read > 0 && line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        
        char* saveptr;
        char* word = strtok_r(line, " ", &saveptr);
        if (word == NULL) {
            continue;
        }

        // Create new word_counters structure
        word_counters_t* wc = mem_malloc(sizeof(word_counters_t));
        if (wc == NULL) {
            free(line);
            index_delete(index);
            fclose(fp);
            return NULL;
        }
        
        // Create counter set
        wc->ctrs = counters_new();
        if (wc->ctrs == NULL) {
            mem_free(wc);
            free(line);
            index_delete(index);
            fclose(fp);
            return NULL;
        }
        
        // Copy word
        wc->word = mem_malloc(strlen(word) + 1);
        if (wc->word == NULL) {
            counters_delete(wc->ctrs);
            mem_free(wc);
            free(line);
            index_delete(index);
            fclose(fp);
            return NULL;
        }
        strcpy(wc->word, word);
        
        // Insert into hashtable
        if (!hashtable_insert(index->ht, wc->word, wc)) {
            mem_free(wc->word);
            counters_delete(wc->ctrs);
            mem_free(wc);
            free(line);
            index_delete(index);
            fclose(fp);
            return NULL;
        }

        // Parse docID-count pairs
        char* id_str;
        while ((id_str = strtok_r(NULL, " ", &saveptr)) != NULL) {
            char* count_str = strtok_r(NULL, " ", &saveptr);
            if (count_str == NULL) {
                break;
            }
            
            int docID = atoi(id_str);
            int count = atoi(count_str);
            if (docID > 0 && count > 0) {
                counters_set(wc->ctrs, docID, count);
            }
        }
    }

    free(line);
    fclose(fp);
    return index;
}

/*
* index_delete(): Frees all memory associated with index
* Params: index pointer to delete (index)
* Returns: void
*/
void index_delete(index_t* index) {
    if (index == NULL) {
        return;
    }
    if (index->ht != NULL) {
        hashtable_delete(index->ht, itemdelete);
    }
    mem_free(index);
}

static void itemdelete(void* item) {
    if (item != NULL) {
        word_counters_t* wc = item;
        if (wc->word != NULL) {
            mem_free(wc->word);
        }
        if (wc->ctrs != NULL) {
            counters_delete(wc->ctrs);
        }
        mem_free(wc);
    }
}