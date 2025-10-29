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

typedef struct wordcount {                                                                                                              
    char *word;                                                                                                                         
    int  frequency;                                                                                                                     
} wordcount_t;  


char *NormalizeWord(char *input);
static bool equals(void *elementp, const void *keyp);
static wordcount_t *wordcount_init(const char *word, int frequency);
static void print_wordcount(void *elementp);
void put_to_file(void* elementp);
static int total_words = 0;
static void sum_frequencies(void *elementp);
static void free_wordcount(void *elementp);



int main(void) {
    webpage_t* loaded_page = pageload(1, "../crawler");
    if (!loaded_page) {
        printf("failed to load page \n");
        return 1;
    }

    FILE *file = fopen("./indexer_output", "w");
    if (!file) { perror("fopen indexer_output"); webpage_delete(loaded_page); return 1; }

    hashtable_t *hmap = hopen(10000);
    if (!hmap) { fprintf(stderr, "hopen failed\n"); fclose(file); webpage_delete(loaded_page); return 1; }

    int pos = 0;
    char *result = NULL;

    while ((pos = webpage_getNextWord(loaded_page, pos, &result)) > 0) {
        char *normalized = NormalizeWord(result);
        if (normalized) {
            fprintf(file, "%s\n", normalized);

            size_t keylen = strlen(normalized) + 1;
            wordcount_t *wc = (wordcount_t *)hsearch(hmap, equals, normalized, keylen);
            if (!wc) {
                wc = wordcount_init(normalized, 1);
                if (wc) hput(hmap, wc, wc->word, keylen);
            } else {
                wc->frequency++;
            }
            free(normalized);  
        }
        free(result);          
    }

    fclose(file);

    happly(hmap, print_wordcount);
    happly(hmap, put_to_file);

    total_words = 0;
    happly(hmap, sum_frequencies);
    printf("TOTAL %d\n", total_words);

    happly(hmap, free_wordcount);
    hclose(hmap);
    webpage_delete(loaded_page);
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
        if (!isalpha(c)) {           
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

static bool equals(void *elementp, const void *keyp) {
    const wordcount_t *wc = (const wordcount_t *)elementp;
    return strcmp(wc->word, (const char *)keyp) == 0;
}

static wordcount_t *wordcount_init(const char *word, int frequency) {
    wordcount_t *wc = malloc(sizeof(wordcount_t));
    if (!wc) return NULL;
    wc->word = malloc(strlen(word) + 1);
    if (!wc->word) {
        fprintf(stderr, "[Error: Malloc failed allocating word string]\n");
        free(wc);
        return NULL;
    }

    strcpy(wc->word, word);
    wc->frequency = frequency;
    return wc;
}

static void print_wordcount(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    printf("%s %d\n", wc->word, wc->frequency);
}

static void sum_frequencies(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    total_words += wc->frequency;
}

static void free_wordcount(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    free(wc->word);
    free(wc);
}

void put_to_file(void* elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    FILE *file = fopen("./indexer_output_hmap", "a");
    fprintf(file, "%s: %d\n", (char *)wc->word, wc->frequency);
    fclose(file);
}
