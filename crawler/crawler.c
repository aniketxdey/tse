/* 
 * crawler.c - Module that crawls webpages up to a specific max depth. See crawler.h for more info.
 * @author: Aniket Dey
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "hashtable.c"
#include "hash.h"
#include "set.h"
#include "mem.h"
#include "webpage.h"
#include "unistd.h"
#include "bag.h"
#include "../common/pagedir.h"

// Function Prototypes
static void parseArgs(const int argc, char* argv[],
                      char** seedURL, char** pageDirectory, int* maxDepth);
static void crawl(char* seedURL, char* pageDirectory, const int maxDepth);
static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen);


/*
 * main: Checks validity with parseArgs and calls main crawl() function
 * Params: number of command-line arguments (argc), array of command-line arguments (argv)
 * Returns: 0 if successful, exits if any errors
 */
int main(const int argc, char* argv[]) {
    char* seedURL; // Points to normalized seed URL
    char* pageDirectory; // Points to path for storing
    int maxDepth = 0; // Max crawling depth

    parseArgs(argc, argv, &seedURL, &pageDirectory, &maxDepth);
    crawl(seedURL, pageDirectory, maxDepth);
    exit(0); // Successful completion of program
}

/*
 * parseArgs: Parses and validates command-line arguments
 * Params: argc, argv, seedURL, pageDirectory, maxDepth
 * Returns: Only returns if arguments are valid; exits otherwise
 */
static void parseArgs(const int argc, char* argv[],
                      char** seedURL, char** pageDirectory, int* maxDepth) {
    // Check validity of usage - four arguments only
    if (argc != 4) {
        fprintf(stderr, "Usage: ./crawler seedURL pageDirectory maxDepth\n");
        exit(1);
    }
    // Normalize the URL; return error if failure 
    char* normURL = normalizeURL(argv[1]); // normalize the URL
    if (normURL == NULL || !isInternalURL(normURL)) {
        fprintf(stderr, "Invalid seedURL\n");
        if (normURL != NULL) {
            mem_free(normURL); // Free memory even if normalization is succesful as long as URL is invalid
        }
        exit(1);
    }
    // Initialize the page directory with pagedir_init; return error if failure
    if (!pagedir_init(argv[2])) {
        fprintf(stderr, "Invalid pageDirectory\n"); // If invalid deictory path, free memory and exit
        mem_free(normURL);
        exit(1);
    }
    // If successful, assign values to pointers
    *seedURL = normURL;
    *pageDirectory = argv[2];
    char* endptr; // Convert maxDepth to an integer
    *maxDepth = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0' || *maxDepth < 0 || *maxDepth > 10) { // If maxDepth is not an integer between 0-10, return an error
        fprintf(stderr, "maxDepth must be an integer between 0 and 10\n"); 
        mem_free(normURL);
        exit(1);
    }
}

/*
 * crawl: Manages the crawling process
 * Params: seedURL (start URL), pageDirectory (directory to write to), maxDepth (to crawl)
 * Returns: None, exits if error
 */
static void crawl(char* seedURL, char* pageDirectory, const int maxDepth) {
    hashtable_t *seen = hashtable_new(200); // Hashtable to keep track of seen pages
    bag_t *pagesToCheck = bag_new();  // Bag to manage pages to crawl
    // Check if the hashtable and bag were created successfully
    if (seen == NULL || pagesToCheck == NULL) { 
        fprintf(stderr, "Failed to create data structures for crawling\n");
        exit(1);
    }
    // Insert seed URL into hashtable to mark as seen
    if (!hashtable_insert(seen, seedURL, "")) {
        fprintf(stderr, "Failed to insert seed URL into hashtable\n");
        bag_delete(pagesToCheck, webpage_delete); // Clean up the bag & hashtable
        hashtable_delete(seen, NULL);
        exit(1);
    }
    // Create a webpage for the seed URL; depth starts at 0
    webpage_t* seedPage = webpage_new(seedURL, 0, NULL);
    if (seedPage == NULL) { // If the webpage ceration fails, clena up bag and hashtable
        fprintf(stderr, "Failed to create webpage for seed URL\n");
        bag_delete(pagesToCheck, webpage_delete);
        hashtable_delete(seen, NULL);
        exit(1);
    }

    
    bag_insert(pagesToCheck, seedPage); // Insert seed webpage into the bag to crawl
    int docID = 1;
    webpage_t* page; // Variable to hold current webpage

    // Loops until there are no more pages to check
    while ((page = bag_extract(pagesToCheck)) != NULL) {
        // Log fetched page with depth and URL
        printf("%d   Fetched: %s\n", webpage_getDepth(page), webpage_getURL(page));

        // Attempt to fetch the content
        if (webpage_fetch(page)) {
            printf("%d  Scanning: %s\n", webpage_getDepth(page), webpage_getURL(page)); // Log the page being scanned
            if (!pagedir_save(page, pageDirectory, docID)) { // Save the fetched page to the page directory
                fprintf(stderr, "Failed to save page with docID %d\n", docID);
                webpage_delete(page); // Clean up the current page, bag, and hashtable before exiting
                bag_delete(pagesToCheck, webpage_delete);
                hashtable_delete(seen, NULL);
                exit(1);
            }
            docID++; // If successfully svaed, increment document ID
            if (webpage_getDepth(page) < maxDepth) { // If the depth is less than maxDepth, for new URLs
                pageScan(page, pagesToCheck, seen);
            }
        } 
        else {
            fprintf(stderr, "Failed to fetch %s\n", webpage_getURL(page));// If fetching page fails, log error 
        }
        webpage_delete(page); // Free memory
    }
    // Clean the bag and hashtable
    bag_delete(pagesToCheck, webpage_delete); 
    hashtable_delete(seen, NULL); 
}


/*
 * pageScan: Extracts URLs from a webpage and adds new internal URLs to the bag
 * Params: page - current web page to crawl, pagesToCrawl - bag of pages to crawls, pagesSeen - hashtable of seen URLs
 * Returns: None
 */
static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen) {
    int position = 0; // Tracks position for extraction
    char* url; // Pointer to hold extracted 
    
    //Loop through all the URLS found in HTML content
    while ((url = webpage_getNextURL(page, &position)) != NULL) {
        char* normalizedURL = normalizeURL(url); // Normalize the URL
        mem_free(url); // Free memory for the old URL

        if (normalizedURL && isInternalURL(normalizedURL)) { // Check validity of the the URL
            if (hashtable_insert(pagesSeen, normalizedURL, "")) {  //Insert into the hastable; if succesful print "found" and "added" messages
                printf("%d     Found: %s\n", webpage_getDepth(page), normalizedURL);
                printf("%d     Added: %s\n", webpage_getDepth(page), normalizedURL);

                webpage_t* newPage = webpage_new(normalizedURL, webpage_getDepth(page) + 1, NULL);
                if (newPage != NULL) {
                    bag_insert(pagesToCrawl, newPage); // Insert a new webPage into teh document to check and crawl later
                } 
                else { 
                    fprintf(stderr, "Failed to create webpage for URL: %s\n", normalizedURL); // IF failure, print an error message and free memory
                    mem_free(normalizedURL);
                }
            } 
            else { // If URL already in hashtable, ignore duplicate and free memory
                printf("%d     Found: %s\n", webpage_getDepth(page), normalizedURL); 
                printf("%d    IgnDupl: %s\n", webpage_getDepth(page), normalizedURL); 
                mem_free(normalizedURL);
            }
        } 
        else if (normalizedURL) { // If URL is external, ignore external and free memoru
            printf("%d     Found: %s\n", webpage_getDepth(page), normalizedURL); 
            printf("%d    IgnExtrn: %s\n", webpage_getDepth(page), normalizedURL);
            mem_free(normalizedURL);
        }
    }
}
