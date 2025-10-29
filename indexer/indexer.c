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


static char *dupstr(const char *s);
char *NormalizeWord(char *input);
static bool equals(void *elementp, const void *keyp);
static wordcount_t *wordcount_init(const char *word);
static void print_wordcount(void *elementp);

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
            /* write occurrence list */
            fprintf(file, "%s\n", normalized);

            /* update hashtable index: word -> frequency */
            size_t keylen = strlen(normalized) + 1;
            wordcount_t *wc = (wordcount_t *)hsearch(hmap, equals, normalized, keylen);
            if (!wc) {
                wc = wordcount_init(normalized);
                if (wc) hput(hmap, wc, wc->word, keylen);
            } else {
                wc->frequency++;
            }
            free(normalized);  // safe because table stores its own strdup
        }
        free(result);          // always free result from webpage_getNextWord
    }

    fclose(file);

    /* print full index (word frequency) */
    happly(hmap, print_wordcount);

    /* sum frequencies and print total */
    total_words = 0;
    happly(hmap, sum_frequencies);
    printf("TOTAL %d\n", total_words);

    /* cleanup */
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

static bool equals(void *elementp, const void *keyp) {
    const wordcount_t *wc = (const wordcount_t *)elementp;
    return strcmp(wc->word, (const char *)keyp) == 0;
}

static wordcount_t *wordcount_init(const char *word) {
    wordcount_t *wc = malloc(sizeof(wordcount_t));
    if (!wc) return NULL;
    wc->word = dupstr(word);              // take our own copy
    if (!wc->word) { free(wc); return NULL; }
    wc->frequency = 1;
    return wc;
}

static void print_wordcount(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    printf("%s %d\n", wc->word, wc->frequency);
}

               // summed after build
static void sum_frequencies(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    total_words += wc->frequency;
}

static void free_wordcount(void *elementp) {
    wordcount_t *wc = (wordcount_t *)elementp;
    free(wc->word);
    free(wc);
}

static char *dupstr(const char *s) {
    char *copy = malloc(strlen(s) + 1);
    if (copy) strcpy(copy, s);
    return copy;
}
