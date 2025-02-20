/* 
* indextest.c - Test program for TSE module on loading and saving.
* @author: Aniket Dey
*/

#include <stdio.h>
#include <stdlib.h>
#include "../common/index.h"

/*
* main(): Validates arguments and tests index load/save operations
* Params: number of command-line arguments (argc), array of command-line arguments (argv)
* Returns: 0 if successful, 1 if any errors occur
*/
int main(int argc, char* argv[]) {
    // Check for correct number of arguments, correct usage if incorrect or none
    if (argc != 3) {
        fprintf(stderr, "Usage: %s oldIndexFilename newIndexFilename\n", argv[0]);
        return 1;
    }
    
    // Get filenames from argument arrau
    char* oldIndexFilename = argv[1];
    char* newIndexFilename = argv[2];
    
    // Load the index from old file
    index_t* index = index_load(oldIndexFilename);
    if (index == NULL) {
        fprintf(stderr, "Error: failed to load index from %s\n", oldIndexFilename);
        return 1;
    }
    
    // Write the index to new file
    if (!index_save(index, newIndexFilename)) {
        fprintf(stderr, "Error: failed to save index to %s\n", newIndexFilename);
        index_delete(index);
        return 1;
    }
    
    // Clean up and exit
    index_delete(index);
    return 0;
}