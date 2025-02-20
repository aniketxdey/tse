/* 
* index.h - Header file for TSE index module
* @author: Aniket Dey
*/

#ifndef INDEX_H
#define INDEX_H

#include <stdbool.h>
#include "../libcs50/counters.h"
#include "../libcs50/hashtable.h"

// Global types
typedef struct index index_t;

// Functions

/*
* index_new(): Creates new index with specified hashtable size
* Params: number of slots for hashtable (num_slots)
* Returns: pointer to new index, or null if error
*/
index_t* index_new(const int num_slots);

/*
* index_add(): Increments word occurrence for given id
* Params: index pointer (index), word to add (word), document ID (id)
* Returns: true if successful, false on error
*/
bool index_add(index_t* index, const char* word, const int id);

/*
* index_set(): Sets exact count for the word-id pair
* Params: index pointer (index), word to set (word), document ID (id), count to set (count)
* Returns: true if successful, false on error
*/
bool index_set(index_t* index, const char* word, const int id, const int count);

/*
* index_get(): Retrieves counters for a given word
* Params: index pointer (index), word to look up (word)
* Returns: pointer to counters set, null if not found
*/
counters_t* index_get(index_t* index, const char* word);

/*
* index_save(): Writes index to file in format
* Params: index pointer (index), filename to write to (filename)
* Returns: true if successful, false on error
*/
bool index_save(index_t* index, const char* filename);

/*
* index_load(): Reads index from file
* Params: filename to read from (filename)
* Returns: pointer to new index, null if error
*/
index_t* index_load(const char* filename);

/*
* index_delete(): Frees all memory associated with index and delete it
* Params: index pointer to delete (index)
* Returns: void
*/
void index_delete(index_t* index);

#endif // INDEX_H