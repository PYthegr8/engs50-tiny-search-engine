/* 
 * indexer.c --- 
 * 
 * Author: Engs 50 25F, Team MergeConflict
 * Created: 10-28-2025
 * Version: 1.0
 * 
 * Description: indexer implementation for tiny search engine
 * 
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "queue.h"
 #include "hash.h"
 #include <unistd.h>
 #include <ctype.h>
 #include "webpage.h"
 #include "pageio.h"



char *NormalizeWord(char *input);

int main() {
    webpage_t* loaded_page = pageload(1, "../crawler");
    FILE *file = fopen("./indexer_output", "w");
    if (loaded_page) {
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextWord(loaded_page, pos, &result)) > 0) {
        char *normalized_word = NormalizeWord(result);
            if (normalized_word) {
                fprintf(file, "%s\n", normalized_word);
                free(normalized_word);
            }
            free(result);
        }
        fclose(file);
        webpage_delete(loaded_page);
    }
    else {
			printf("failed to load page \n");
    }
	return 0;
}

char *NormalizeWord(char *input){
     int len = strlen(input);
     if (len < 3) {
        printf("word less than 3 characters \n");
        return NULL;
     }

     char *newWord = malloc(len + 1);

     if (newWord == NULL) {
        return NULL;
     }

     int i = 0;
     int j = 0;
     while (input[i] != '\0') {
         unsigned char c = (unsigned char)input[i];
         if (isalnum(c)) {
             newWord[j++] = (char)tolower(c);
         }
         i++;
     }
     newWord[j] = '\0';

     if (j < 3) {
         free(newWord);
         return NULL;
     }
     return newWord;
}
