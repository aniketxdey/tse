/* 
 * set.c - Module for the set data structure.
 * A set maintains an unordered collection of pairs with unique keys.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "set.h"
#include "mem.h"

// Local Types

/* 
 * setnode: struct representing a single node in the set
 */
typedef struct setnode {
    char* key; // key for this item (string)
    void* item;              
    struct setnode* next; // pointer to the next node
} setnode_t;

// Global Types

/* 
 * set: Struct representing the entire set
 */
typedef struct set {
    struct setnode* head; // head of the linked list
} set_t;

// Prototypes

static setnode_t* setnode_new(void* item, const char* key);

// Functions

/*
 * set_new - Create a new (empty) set.
 * Returns pointer to a new set, or NULL if error. Remember to delete set to protect malloc.
 */
set_t* set_new(void) {
    set_t* set = malloc(sizeof(set_t)); //create space in memory for the new set struct
    if (set == NULL) {
        return NULL; //If malloc fails, return null
    } else {
        set->head = NULL; // Initialize the set
        return set;
    }
}

/*
 * set_insert - Given a specific key, insert an item into the set
 * Params: pointer to set to insert into, constant value for key, item to be inserted
 * Returns true if new item was inserted successfully, amd false if key already exists in set.
 * Also returns false on null parameters and errors.
 */
bool set_insert(set_t* set, const char* key, void* item) {
    if (set == NULL || key == NULL || item == NULL) {
        return false; // Handles all invalid parameters (null key, set, item)
    }
    // Loop through the set and check if the key already exists.
    for (setnode_t* node = set->head; node != NULL; node = node->next) { 
        if (strcmp(node->key, key) == 0) {
            return false; // Key already exists, insertion fails and return false
        }
    }
    // Create a new node if the key is new
    setnode_t* new_node = setnode_new(item, key);
    if (new_node == NULL) {
        return false; //If malloc fails, return false
    }
    // Insert the new node at the head of the list
    new_node->next = set->head;
    set->head = new_node;
    return true;
}

/*
 * set_find: Given a key, return the item associated.
 * Params: Constant key value and pointer to set to look at
 * Returns a pointer to the item corresponding to the given key, or null if key not found/set is null.
 */
void* set_find(set_t* set, const char* key) {
    if (set == NULL || key == NULL) {
        return NULL; // // Handles all invalid parameters (null key, set)
    }
    // Now, search for the key in the set
    for (setnode_t* node = set->head; node != NULL; node = node->next) {
        if (strcmp(node->key, key) == 0) {
            return node->item; // If the key is found, return the corresponding item
        }
    }
    return NULL;
}

/*
 * set_print - Print the whole set to the given file path.
 * Params: pointer to set, file pointer/path, itemprint function 
 * Return: Void, but prints the set in {key:item} format. If itemprint null, print only keys.
 */
void set_print(set_t* set, FILE* fp, 
               void (*itemprint)(FILE* fp, const char* key, void* item)) {
    if (fp != NULL) { //check fp 
        if (set == NULL) { //check for null set
            fputs("(null)", fp); //Put null for null set 
        } 
        else {
            fputs("{", fp);
            for (setnode_t* node = set->head; node != NULL; node = node->next) { // Iterate over the set and print each item
                if (itemprint != NULL) {
                    (*itemprint)(fp, node->key, node->item);
                } else {
                    fprintf(fp, "%s", node->key); // If itemprint is null, only print the keys
                }
                if (node->next != NULL) {
                    fputs(", ", fp);
                }
            }
            fputs("}", fp);
        }
    }
}


/*
 * set_iterate - Iterate over all items in the set, calling the given function on each item.
 * Params: pointer to set, pointer to arg, and itemfunc
 * Returns: Void, but iterates through set.
 */
void set_iterate(set_t* set, void* arg, 
                 void (*itemfunc)(void* arg, const char* key, void* item)) {
    
    if (set != NULL && itemfunc != NULL) { // Handle invalid parameters (null set, itemfunc)
        for (setnode_t* node = set->head; node != NULL; node = node->next) {
            (*itemfunc)(arg, node->key, node->item); // Call itemfunc on each item
        }
    }
}

/*
 * set_delete - Delete the whole set; ignore NULL set.
 * Params: pointer to set, itemdelete function 
 * Returns: Void, but deletes and frees memory.
 */
void set_delete(set_t* set, void (*itemdelete)(void* item)) {
    if (set != NULL) {
        // Iterate over the set and delete each node to free memory
        for (setnode_t* node = set->head; node != NULL;) {
            setnode_t* next = node->next;
            if (itemdelete != NULL) {
                (*itemdelete)(node->item);
            }
            free(node->key);
            free(node); // Free the node after the key
            node = next;
        }
        free(set); // Free the set structure
    }
}

// Local funcs/helper methods

/*
 * setnode_new: Given a key and item, creates a new node.
 * Returns pointer to new node, or null if malloc fails. 
 */
static setnode_t* setnode_new(void* item, const char* key) {
    // Allocate memory for the node
    setnode_t* node = malloc(sizeof(setnode_t));
    if (node == NULL) {
        return NULL; // Malloc fails, return null
    }
    // Allocate memory and copy  key
    char* copy = malloc(strlen(key) + 1);
    if (copy == NULL) {
        free(node); // Malloc fails, return null
        return NULL;
    }
    strcpy(copy, key); 
    // Initialize the node
    node->key = copy;
    node->item = item;
    node->next = NULL;
    return node;
}
