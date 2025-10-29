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
    if (loaded_page) {
			  FILE *file = fopen("./indexer_output", "w"); 
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
        return NULL;
     }

     char *newWord = malloc(len + 1);

     if (newWord == NULL) {
        return NULL;
     }

     for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (!isalpha(c)) {           // <-- reject whole word if any non-letter
            free(newWord);
            return NULL;
        }
        newWord[i] = (char)tolower(c);
     }
		 
		 newWord[len] = '\0';
		 
     if (len < 3) {
         free(newWord);
         return NULL;
     }
     return newWord;
}
