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

int main(){
    webpage_t* page = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    if(webpage_fetch(page)) {

        char *html = webpage_getHTML(page);
        printf("Found html: %s\n", html);


        //finding URLs in page
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextURL(page, pos, &result)) > 0) {
            (IsInternalURL(result)) ? printf("Found an internal URL: %s\n", result) : printf("Found an external URL: %s\n", result);
            free(result);
         }

        //deallocating webpage
        webpage_delete(page);
        return EXIT_SUCCESS;
    }

    else {
        printf("Could not fetch webpage\n");
        return EXIT_FAILURE;
    }
}


