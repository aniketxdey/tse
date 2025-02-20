/* 
* pagedir.h - Header file for 'pagedir.c' module
* @author: Aniket Dey
*/

#ifndef PAGEDIR_H
#define PAGEDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "../libcs50/webpage.h"

/*
* pagedir_init: Initializes directory to store pages with .crawler file
* Params: pageDirectory - path to the directory to initialize
* Returns: true is successful, false otherwise
*/
bool pagedir_init(const char* pageDirectory);

/*
* pagedir_save: Saves a webpage to a file in the page directory
* Params: page - pointer to the webpage, pageDirectory - path to save to, id - document ID for page
* Returns: true if the page is successfully saved, false otherwise
*/
bool pagedir_save(const webpage_t* page, const char* pageDirectory, const int id);

/*
* pagedir_validate: Checks if directory is a crawler-produced directory
* Params: pageDirectory - path to the directory to validate
* Returns: true if directory contains .crawler filetype, false otherwise
*/
bool pagedir_validate(const char* pageDirectory);

/*
* pagedir_load: Loads webpage from file in directory
* Params: pageDirectory - directory containing page files, id - ID of page to load
* Returns: pointer to webpage if successful, null otherwise
*/
webpage_t* pagedir_load(const char* pageDirectory, const int id);

#endif // PAGEDIR_H