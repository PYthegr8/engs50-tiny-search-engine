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
 #include <stdbool.h>
 #include "queue.h"
 #include "hash.h"
 #include <unistd.h>
 #include <ctype.h>
 #include "webpage.h"
 #include "pageio.h"

typedef struct wordcount {
    char *word;
    int frequency;
} wordcount_t;

bool NormalizeWord(char *newWord, char *input);
bool key_exist(hashtable_t *htp, void* keyp);
bool equals(void* elementp, const void* keyp);
wordcount_t *wordcount_init(char *word, int frequency);
void put_to_file(void* elementp);

int main() {
    webpage_t* loaded_page = pageload(1, "../crawler");
    FILE *file = fopen("./indexer_output", "w");
    hashtable_t *hmap = hopen(10000);
    if (loaded_page) {
        int pos = 0;
        char *result;
        while ((pos = webpage_getNextWord(loaded_page, pos, &result)) > 0) {
            int len = strlen(result);
            char *wordToBeNormalized = (char*)malloc(sizeof(char) * len + 1);
            bool success = NormalizeWord(wordToBeNormalized, result);
                if (success) {
                    fprintf(file, "%s\n", wordToBeNormalized);
                    if (!key_exist(hmap, wordToBeNormalized)) {
                        wordcount_t *wc = wordcount_init(wordToBeNormalized, 1);
                        hput(hmap, (void*)wc, wordToBeNormalized, len);
                    }
                    else {
                        wordcount_t *wc = (wordcount_t *)hsearch(hmap, equals, wordToBeNormalized, len);
                        wc->frequency++;
                    }
                }
                free(result);
                free(wordToBeNormalized);
            }
        happly(hmap, put_to_file);
        hclose(hmap);
        fclose(file);
        webpage_delete(loaded_page);
    }
    else {
			printf("failed to load page \n");
    }
	return 0;
}

bool NormalizeWord(char *newWord, char *input){
     int len = strlen(input);
     if (len < 3) {
        printf("word less than 3 characters \n");
        return false;
     }

     int i = 0;
     while (input[i] != '\0') {
         unsigned char c = (unsigned char)input[i];
         if (!isalnum(c)) {
             return false;
         }
         newWord[i++] = (char)tolower(c);
     }
     newWord[i] = '\0';

     if (i < 3) {
         free(newWord);
         return false;
     }
     return true;
}

bool key_exist(hashtable_t *htp, void* keyp) {
    size_t keylen = strlen((const char *)keyp) + 1;
    if((hsearch(htp, equals, keyp, keylen)) == NULL) {
        return false;
    }
    return true;
}

bool equals(void* elementp, const void* keyp) {
    const char *compare_word = (const char *)elementp;
    const char *hmap_key = (const char *)keyp;
    if (!strcmp(compare_word, hmap_key)) {
        return true;
    }
    return false;
}

wordcount_t *wordcount_init(char *word, int frequency) {
    wordcount_t *wc = malloc(sizeof(wordcount_t));    
    if (!wc) {
        fprintf(stderr, "[Error: Malloc failed allocating wordcount struct]\n");
        return NULL;
    }
    wc->word = word;
    wc->frequency = frequency;
    return wc;
}   

void put_to_file(void* elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    FILE *file = fopen("./indexer_output_hmap", "a");
    fprintf(file, "%s\n", (char *)wc->word);
    fclose(file);
}
