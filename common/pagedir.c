/* 
* pagedir.c - Module that handles page directory initialization and saving webpages.
* @author: Aniket Dey
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "../libcs50/webpage.h"
#include "../libcs50/file.h"
#include "pagedir.h"
#include "../libcs50/mem.h"

/*
* pagedir_init: Initializes directory to store pages with .crawler file
* Params: pageDirectory - path to the directory to initialize
* Returns: true is successful, false otherwise
*/
bool pagedir_init(const char* pageDirectory) {
    if (pageDirectory == NULL) {
        return false; 
    }

    // Malloc for the path to the .crawler file
    char* path = mem_malloc(strlen(pageDirectory) + strlen("/.crawler") + 1);
    if (path == NULL) {
        return false;
    }

    strcpy(path, pageDirectory);
    strcat(path, "/.crawler");

    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        mem_free(path);
        return false;
    }
    
    fclose(fp);
    mem_free(path); 
    return true; 
}

/*
* pagedir_save: Saves a webpage to a file in the page directory
* Params: page - pointer to the webpage, pageDirectory - path to save to, docID - document ID for page
* Returns: true if the page is successfully saved, false otherwise
*/
bool pagedir_save(const webpage_t* page, const char* pageDirectory, const int docID) {
    if (page == NULL || pageDirectory == NULL || docID < 1) {
        return false; 
    }
    
    // Allocate and build the complete path
    int pathLength = snprintf(NULL, 0, "%s/%d", pageDirectory, docID) + 1;
    char* path = mem_malloc(pathLength);
    if (path == NULL) {
        return false;
    }

    snprintf(path, pathLength, "%s/%d", pageDirectory, docID);
    
    // Try to open the file
    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        mem_free(path);
        return false; 
    }

    // Write the page contents
    fprintf(fp, "%s\n%d\n%s", webpage_getURL(page), webpage_getDepth(page), webpage_getHTML(page));
    
    // Clean up
    fclose(fp);
    mem_free(path);
    return true;
}

/*
* pagedir_validate: Checks if directory is a crawler-produced directory
* Params: pageDirectory - path to the directory to validate
* Returns: true if directory contains .crawler file, false otherwise
*/
bool pagedir_validate(const char* pageDirectory) {
    if (pageDirectory == NULL) {
        return false;
    }
    
    // Create the full path to .crawler file
    char* path = mem_malloc(strlen(pageDirectory) + strlen("/.crawler") + 1);
    if (path == NULL) {
        return false;
    }
    
    strcpy(path, pageDirectory);
    strcat(path, "/.crawler");
    
    // Try to open the file
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        mem_free(path);
        return false;
    }
    
    fclose(fp);
    mem_free(path);
    return true;
}

/*
* pagedir_load: Loads webpage from file in directory
* Params: pageDirectory - directory containing page files, docID - ID of page to load
* Returns: pointer to webpage if successful, null otherwise
*/
webpage_t* pagedir_load(const char* pageDirectory, const int docID) {
    if (pageDirectory == NULL || docID < 1) {
        return NULL;
    }
    
    // Create the full path
    int pathLength = snprintf(NULL, 0, "%s/%d", pageDirectory, docID) + 1;
    char* path = mem_malloc(pathLength);
    if (path == NULL) {
        return NULL;
    }
    
    snprintf(path, pathLength, "%s/%d", pageDirectory, docID);
    
    // Try to open the file
    FILE* fp = fopen(path, "r");
    mem_free(path); // Done with path regardless of fopen result
    
    if (fp == NULL) {
        return NULL;
    }

    // Read the page components
    char* url = file_readLine(fp);
    char* depth_str = file_readLine(fp);
    char* html = file_readFile(fp);
    
    // Validate all components
    if (url == NULL || depth_str == NULL || html == NULL) {
        // Clean up any successfully allocated components
        if (url != NULL) free(url);
        if (depth_str != NULL) free(depth_str);
        if (html != NULL) free(html);
        fclose(fp);
        return NULL;
    }

    // Convert depth string to integer
    int depth = atoi(depth_str);
    free(depth_str);
    
    // Create webpage which now owns the url and html; don't free!
    webpage_t* page = webpage_new(url, depth, html);
    fclose(fp);
    return page;
}