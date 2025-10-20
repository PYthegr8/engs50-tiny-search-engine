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
int32_t pagesave(webpage_t *pagep, int id, char *dirname);
void usage(char *progname);
int validate_pagedir(char *pagedir);
int crawl(char *seedurl, char *pagedir, int maxdepth);



int main(int argc, char *argv[]){
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
                if (hput(lookup, (void*)new_page, result, strlen(result) + 1)) {
                    fprintf(stderr, "[Error: Could not insert the URL into the queue]\n"); 
                    free(result);
                    free(new_page);
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

    // Module 4 step 5
    printf("Saving webpage contents to a file");
    webpage_t* page_step5 = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    if(webpage_fetch(page_step5)) {
        pagesave(page_step5, 1, "../pages");
    }
    webpage_delete(page_step5);

		// Module 4 step 6
	  if (argc != 4) {
			usage(argv[0]);
			return EXIT_FAILURE;
    }

	  char *seedurl = argv[1];
    char *pagedir = argv[2];
    int maxdepth = atoi(argv[3]);

    return crawl(seedurl, pagedir, maxdepth);
	
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
    size_t keylen = strlen((const char *)keyp) + 1;
    if((hsearch(htp, equals, keyp, keylen)) == NULL) {
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

int32_t pagesave(webpage_t *pagep, int id, char *dirname) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirname, id);

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        fprintf(stderr, "[Error: Could not open file %s]\n", filepath);
        return EXIT_FAILURE;
    }

    fprintf(file, "%s\n", webpage_getURL(pagep));
    fprintf(file, "%d\n", webpage_getDepth(pagep));
    fprintf(file, "%d\n", webpage_getHTMLlen(pagep));

    // optionally write the HTML content
    fprintf(file, "%s", webpage_getHTML(pagep));

    fclose(file);
    return EXIT_SUCCESS;
}

void usage(char *progname) {
    fprintf(stderr, "usage: %s <seedurl> <pagedir> <maxdepth>\n", progname);
}

int validate_pagedir(char *pagedir) {
    char path[512];
    sprintf(path, "%s/.crawler", pagedir);

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return -1;   // directory not writable or doesn’t exist
    }
    fclose(f);
    remove(path);
    return 0;
}

int crawl(char *seedurl, char *pagedir, int maxdepth) {
    if (!IsInternalURL(seedurl)) {
        fprintf(stderr, "Error: seedurl must be an internal URL.\n");
        return -1;
    }
    if (validate_pagedir(pagedir) != 0) {
        fprintf(stderr, "Error: pagedir '%s' not writable or does not exist.\n", pagedir);
        return -1;
    }
    if (maxdepth < 0) {
        fprintf(stderr, "Error: maxdepth must be non-negative.\n");
        return -1;
    }

    queue_t     *q = qopen();
    hashtable_t *visited = hopen(10000);
    if (!q || !visited) {
        fprintf(stderr, "Error: could not allocate crawler structures.\n");
        if (q) qclose(q);
        if (visited) hclose(visited);
        return -1;
    }

    webpage_t *page_step6 = webpage_new((char *)seedurl, 0, NULL);
    if (!page_step6) {
        fprintf(stderr, "Error: could not allocate seed page.\n");
        qclose(q);
        hclose(visited);
        return -1;
    }

    hput(visited, (void *)page_step6, (char *)seedurl, (int)strlen(seedurl) + 1);
    qput(q, (void *)page_step6);

    int docID = 1;
    int saved = 0;

    for (webpage_t *curr = qget(q); curr != NULL; curr = qget(q)) {
        if (!webpage_fetch(curr)) {
            continue;  // skip failed fetches
        }

        if (pagesave(curr, docID, (char *)pagedir) == EXIT_SUCCESS) {
            docID++;
            saved++;
        }

        int depth = webpage_getDepth(curr);
        if (depth < maxdepth) {
            int pos = 0;
            char *found = NULL;
            while ((pos = webpage_getNextURL(curr, pos, &found)) > 0) {
                if (IsInternalURL(found) && !key_exist(visited, found)) {
                    webpage_t *child = webpage_new(found, depth + 1, NULL);
                    if (child) {
                        qput(q, (void *)child);
                        hput(visited, (void *)child, found, (int)strlen(found) + 1);
                    }
                }
                free(found);
            }
        }
    }

    printf("[crawler] Crawl complete. Saved %d pages.\n", saved);

    happly(visited, free_page);
    hclose(visited);
    qclose(q);
    return 0;
}
