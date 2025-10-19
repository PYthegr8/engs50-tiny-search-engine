/* 
 * crawler.c --- 
 * 
 * Author: cs50 Team MergeConflict
 * Created: 10-17-2025
 * Version: 1.0
 * 
 * Description: This program retrieves the  HTML page from  https://thayer.github.io/engs50/ and prints out all the URLs it contains
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webpage.h>
#include "queue.h"
#include "hash.h"

void print_q_contents(void* elementp);
void free_page(void* elementp);
bool key_exist(hashtable_t *htp, void* keyp);
bool equals(void* elementp, const void* keyp);

int main(){
    // Module 4 Step 2
    printf("Not using any datastructures\n");
    webpage_t* page = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    if(webpage_fetch(page)) {

        //char *html = webpage_getHTML(page);
        //printf("Found html: %s\n", html);


        //finding URLs in page
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextURL(page, pos, &result)) > 0) {
            (IsInternalURL(result)) ? printf("Found an internal URL: %s\n", result) : printf("Found an external URL: %s\n", result);
            free(result);
         }

        //deallocating webpage
        webpage_delete(page);
        //return EXIT_SUCCESS;
    }

    else {
        printf("Could not fetch webpage\n");
        return EXIT_FAILURE;
    }
    // Module 4 step 3
    printf("Using a queue to place all the webpages we fetch\n");
    webpage_t* page_step3 = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    queue_t *q = qopen();

    if(webpage_fetch(page_step3)) {

        int pos = 0;
        char *result;
        while ((pos = webpage_getNextURL(page_step3, pos, &result)) > 0) {
            if (IsInternalURL(result)) {
                webpage_t* new_page = webpage_new(result, 0, NULL);
                if (qput(q, (void*)new_page)) {
                    fprintf(stderr, "[Error: Could not insert the URL into the queue]\n"); 
                    return EXIT_FAILURE;
                }
            }
            free(result);
        }
        qapply(q, print_q_contents);
    }
    webpage_delete(page_step3);
    qapply(q, free_page);
    qclose(q);

    // Module 4 step 4
    printf("Using a hashtable to eliminte duplicate URLs\n");
    webpage_t* page_step4 = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    hashtable_t *lookup = hopen(1000);

    if(webpage_fetch(page_step4)) {
        
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextURL(page_step4, pos, &result)) > 0) {
            printf("This is the current URL we are getting: %s\n", result);
            if (IsInternalURL(result) && !key_exist(lookup, result)) {
                webpage_t* new_page = webpage_new(result, 0, NULL);
                if (hput(lookup, (void*)new_page, result, sizeof(result))) {
                    fprintf(stderr, "[Error: Could not insert the URL into the queue]\n"); 
                    return EXIT_FAILURE;
                }
            }
            free(result);
        }
        happly(lookup, print_q_contents);
    }
    webpage_delete(page_step4);
    happly(lookup, free_page);
    hclose(lookup);
}


void print_q_contents(void* elementp) {
    webpage_t* page = (webpage_t*)elementp;
    printf("URL: %s\n", webpage_getURL(page));
}

void free_page(void* elementp) {
    webpage_t* page = (webpage_t*)elementp;
    webpage_delete(page);
}

bool key_exist(hashtable_t *htp, void* keyp) {
    if ((hsearch(htp, equals, keyp, sizeof(keyp))) == NULL) {
        return false; 
    }
    return true;
}

bool equals(void* elementp, const void* keyp) {
    webpage_t* page = (webpage_t*)elementp;
    const char* url = webpage_getURL(page);
    const char* keyURL = (const char*)keyp;

    printf("URL we're passing in is: %s\n", url);
    printf("URL we are comparing with is: %s\n", keyURL);

    if (strcmp(url, keyURL) == 0) {  // compare string contents
        return true;
    }

    return false;
}

