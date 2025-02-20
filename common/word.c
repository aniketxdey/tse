/*
* word.c - Word module for TSE, provides functions for normalizing words.
* @author: Aniket Dey
*/

#include <ctype.h>
#include <string.h>
#include "word.h"

/*
* normalizeWord(): Converts all characters in word to lowercase.
* Params: pointer to word string to normalize (word)
* Returns: void
*/
void normalizeWord(char* word) {
    if (word == NULL) { // Check for null pointer
        return;
    }
    // Convert each character to lowercase until end of string
    for (int i = 0; word[i] != '\0'; i++) {
        word[i] = tolower(word[i]);
    }
}