/* 
 * counters.c - Module for the counter set data structure
 * @author: Aniket Dey
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "counters.h"
#include "mem.h"

// Local types
typedef struct counternode {
    int item; // Key value for the node
    struct counternode* next; // Pointer to the next node
    int count;  
} counternode_t;

// Global types
typedef struct counters {
    struct counternode* head;
} counters_t;

// Local function prototypes
static counternode_t* counternode_new(const int key);


/*
* counters_new: Creates and initializes a new counters structure
* If malloc fails, returns NULL. Else, returns a pointer.
*/
counters_t* counters_new(void){
    counters_t* counters = mem_malloc(sizeof(counters_t));
    if (counters == NULL){
        return NULL; // Test malloc to see if it fails
    }
    else {
        counters->head = NULL;
        return counters;
    }
} 

// Functions (as described in assignment)

/* 
* counters_add: If the key exists, increment the count for it. Else, create a new count.
* Returns the new count, or 0 if an error is detected.
*/
int counters_add(counters_t* ctrs, const int key){
    if (ctrs == NULL || key < 0){
        return 0;
    }
    if (ctrs->head == NULL){
        ctrs->head = counternode_new(key);
        if (ctrs->head == NULL) {
            return 0; // Test to see if malloc fails
        }
        return ctrs->head->count;
    }
    else {
        counternode_t* prev = NULL;
        for(counternode_t* node = ctrs->head; node != NULL; node = node->next){ // Loop through each node
           
            if (key == node->item){
                node->count++;
                return node->count; 
            }
            prev = node;    
        }
        counternode_t* new = counternode_new(key);
        if (new == NULL){
            // Memory allocation failed
            return 0;
        }
        prev ->next = new;
        return new->count; 
    }
}

/*
* counternode_new: Helper method for _add and _set; 
* Given a key, it returns a pointer to the new node, or null if the memory allocation fails
*/
static counternode_t* counternode_new(const int key){
    counternode_t* node = mem_malloc(sizeof(counternode_t));

    if (node == NULL) {
        return NULL; // If malloc fails, returns null
    } 
    else { // Initialize the new node
        node->item = key;
        node->next = NULL;
        node->count = 1;
        return node;
    }
}

/*
* counters_get: Returns the current count for the given key.
* If does not exist/ctrs is null - returns the count
*/
int counters_get(counters_t* ctrs, const int key){
    if (ctrs == NULL || key < 0){
        return 0;
    }
    else {
        for(counternode_t* node = ctrs->head; node != NULL; node = node->next){ // Loop through the counterset
            if (node->item == key){
                return node->count;
            }
        }
        return 0;
    }
}


/*
* counters_set: Given a count and a key, set the counter.
* If the key doesn't exist, create a new counter
* Returns true on successful completion, false if errors in malloc/ctrs
*/
bool counters_set(counters_t* ctrs, const int key, const int count){
    if (ctrs == NULL || key < 0 || count < 0){
        return false; // Defensive coding
    }
    else{
        counternode_t* prev = NULL;
        for(counternode_t* node = ctrs->head; node != NULL; node = node->next){ // Loop through the nodes
            if (key == node->item){
                node->count = count; // If the key exists, increment the count
                return true;
            }  
            prev = node;
        }
        counternode_t* new = counternode_new(key); // Key not found, create a new node
        if (new == NULL){
            return false; 
        }
        new->count = count;
        if (prev == NULL){

            ctrs->head = new; // IF list was empty, set new node as head
        } else {
            prev->next = new;
        }
        return true; 
    }
}

/*
* counters_print: Print the counters set to the file "fp"
* Returns void
*/
void counters_print(counters_t* ctrs, FILE* fp){
    if (fp != NULL){
        if (ctrs == NULL){
            fputs("(null)", fp);
        }
        else { // If no errors, look through the counter set to print to the file 
            fputs("{", fp);
            for(counternode_t* node = ctrs->head; node != NULL; node = node->next){
                fprintf(fp, "%d:%d", node->item, node->count);
                if (node->next != NULL){
                    fputs(", ", fp);
                }
            }
            fputs("}",fp);
        }   
    }
}

/*
* counters_iterate: Given a count and a key, set the counter.
* Returns void
*/
void counters_iterate(counters_t* ctrs, void* arg, void (*itemfunc)(void* arg, const int key, const int count)){
    if (ctrs != NULL && itemfunc != NULL){
        // Call itemfunc with arg, on each item
        for (counternode_t* node = ctrs->head; node != NULL; node = node->next) {
            (*itemfunc)(arg, node->item, node->count); 
        }
    }
}

/*
* counters_delete: Deletes full counter set and frees memory for allocation
*/
void counters_delete(counters_t* ctrs){
    if (ctrs != NULL){
        for (counternode_t* node = ctrs->head; node != NULL;){
            counternode_t* next = node->next;
            mem_free(node);
            node = next;
        }
        mem_free(ctrs);
    }
}
