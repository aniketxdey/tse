/* 
 * hashtable.c - Module for the hashtable datastructure
 * See hashtable.h for more information.
 * @auhtor: Aniket Dey
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "hash.h"
#include "../libcs50/set.h"
#include "mem.h"

// Local Types

// hashtable struct represents the entire hashtable.
typedef struct hashtable {
    int num_slots; // number of slots in the table
    set_t** table; // array of pointers to sets
} hashtable_t;

// Functions

/*
 * hashtable_new: Create a new hashtable with the given number of slots.
 * Params: number of slots in the hashtable.
 * Returns a pointer to a new hashtable, or null on error.
 */
hashtable_t* hashtable_new(const int num_slots) {
    if (num_slots <= 0) {
        return NULL; // Handle invalid parameter (null # of slots)
    }
    // Allocate memory
    hashtable_t* ht = malloc(sizeof(hashtable_t));
    if (ht == NULL) {
        return NULL; // If malloc fails, return null
    }
    ht->num_slots = num_slots;

    // Allocate memory for the array of set pointers
    ht->table = calloc(num_slots, sizeof(set_t*));
    if (ht->table == NULL) {
        free(ht);
        return NULL; // If malloc fails, return null and free ht
    }
    // Initialize each slot with a new set
    for (int i = 0; i < num_slots; i++) {
        ht->table[i] = set_new();
        if (ht->table[i] == NULL) {
            for (int j = 0; j < i; j++) {
                set_delete(ht->table[j], NULL); // If the intialization fails, delete and clean up
            }
            free(ht->table);
            free(ht);
            return NULL;
        }
    }

    return ht; // Successfully created hashtable
}

/*
 * hashtable_insert - Insert item into the hashtable under the given key.
 * Params: pointer to hashtable, constant string key to insert, item to be inserted
 * Returns true if new item was inserted successfully, andfalse if key already exists in hashtable.
 * Also returns false on null  parameters or errors.
 */
bool hashtable_insert(hashtable_t* ht, const char* key, void* item) {
    if (ht == NULL || key == NULL || item == NULL) {
        return false; // Handle invalid parameters (null ht, key, item)
    }
    // Compute hash value for the given key and insert
    int hash = hash_jenkins(key, ht->num_slots);
    return set_insert(ht->table[hash], key, item);
}

/*
 * hashtable_find - Return the item associated with the given key.
 * Params: pointer to hashtable, constant string key to find
 * Returns pointer to the item corresponding to the given key, or NULL if key not found or ht is NULL.
 */
void* hashtable_find(hashtable_t* ht, const char* key) {
    if (ht == NULL || key == NULL) {
        return NULL; // Handle invalid parameters (null ht, key)
    }
    // // Compute hash value for the given key and find in the set
    int hash = hash_jenkins(key, ht->num_slots);
    return set_find(ht->table[hash], key);
}

/*
 * hashtable_print - Print the whole hashtable to the given file.
 * Params pointer to hashtable, file pointer to print to , itemprint function
 * Returns: Nothing, but prinys table. Prints keys if itemprint is null.
 */
void hashtable_print(hashtable_t* ht, FILE* fp, void (*itemprint)(FILE* fp, const char* key, void* item)) {
    if (fp != NULL) { // Check if file pointer is valid
        if (ht == NULL) {
            fputs("(null)", fp); // Print null if table is null
        } else {
            // Iterate over each slot in the hashtable
            for (int i = 0; i < ht->num_slots; i++) {
                fprintf(fp, "Slot %d: ", i);
                if (itemprint == NULL) {
                    set_print(ht->table[i], fp, NULL); // If itemprint does not exist or is nukll, print only the keys
                } else {
                    // Else, print with itemprint()
                    set_print(ht->table[i], fp, itemprint);
                }
                fprintf(fp, "\n");
            }
        }
    }
}

/*
 * hashtable_iterate - Iterate over all items in the hashtable.
 * Params: pointer to hashtable, pointer for data to itemfunc, itemfunc to apply to all items
 * Returns nothing, but iterates through.
 */
void hashtable_iterate(hashtable_t* ht, void* arg, void (*itemfunc)(void* arg, const char* key, void* item)) {
    if (ht != NULL && itemfunc != NULL) { // Handle invalid parameters (null ht, item func)
        // Iterate over each slot in the hashtable, and look at each set
        for (int i = 0; i < ht->num_slots; i++) {
            set_iterate(ht->table[i], arg, itemfunc);
        }
    }
}

/*
 * hashtable_delete - Delete the whole hashtable; ignore NULL ht.
 * Params: pointer to hashtable, function to delete each item 
 */
void hashtable_delete(hashtable_t* ht, void (*itemdelete)(void* item)) {
    if (ht != NULL) {
        // Iterate over each slot in the hashtable and delete the sets
        for (int i = 0; i < ht->num_slots; i++) {
            set_delete(ht->table[i], itemdelete);
        }
        // Free the array of set pointers and structure at the end
        free(ht->table);
        free(ht);
    }
}
