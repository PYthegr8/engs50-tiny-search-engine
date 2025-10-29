/* 
* pageio.c --- 
* 
* Author: Papa Yaw Owusu Nti
* Created: 10-28-2025
* Version: 1.0
* 
* Description: 
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "webpage.h"
#include "pageio.h"


int32_t pagesave(webpage_t *pagep, int id, char *dirnm);
webpage_t *pageload(int id, char *dirnm);

int32_t pagesave(webpage_t *pagep, int id, char *dirnm) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirnm, id);

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

/*
 * pageload -- loads the numbered filename <id> in direcory <dirnm>
 * into a new webpage
 *
 * returns: non-NULL for success; NULL otherwise
 */
webpage_t *pageload(int id, char *dirnm) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirnm, id);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "[Error: Could not open the file at filepath: %s]\n", filepath);
        return NULL;
    }
    char url[256];
    if (fgets(url, sizeof(url), file)) {
        printf("First Line: %s\n", url);
    }

    if (strlen(url) > 0 && url[strlen(url) - 1] == '\n') {
        url[strlen(url) - 1] = '\0';
    }

    int depth;

    if ((depth = fgetc(file)) == EOF) {
        fprintf(stderr, "[Error: Could not get the depth of the webpage]\n");
        return NULL;
    }

    webpage_t *new_webpage;
    if(!(new_webpage = webpage_new(url, depth, NULL))) {
        fprintf(stderr, "[Error: Could not create a new webpage\n");
        return NULL;
    }

    if (!webpage_fetch(new_webpage)) {
        fprintf(stderr, "[Error: Could not fetch the html of the webpage from the url!]\n");
        return NULL;
    }

    fclose(file);
    return new_webpage;
}
