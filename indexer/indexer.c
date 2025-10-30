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
#include "indexio.h"

typedef struct wordcount {                                                                                                              
    char *word;                                                                                                                         
    int  frequency;                                                                                                                     
} wordcount_t;  

typedef struct {
    int docID;
    int count;
} posting_t;

typedef struct {
    char    *word;   
    queue_t *plist;  
} wordentry_t;




char *NormalizeWord(char *input);
static bool equals(void *elementp, const void *keyp);
static wordcount_t *wordcount_init(const char *word, int frequency);
static void print_wordcount(void *elementp);
void put_to_file(void* elementp);
static int total_words = 0;
static void sum_frequencies(void *elementp);
static void free_wordcount(void *elementp);
static char *xstrdup(const char *s);
static bool word_equals(void *elementp, const void *keyp);
static bool doc_equals(void *elementp, const void *keyp);
static wordentry_t *wordentry_init(const char *word);
static posting_t *posting_init(int docID);
static int index_update_doc(hashtable_t *ht, const char *norm_word, int docID);

static FILE *g_out_multi = NULL; // context for apply callbacks
static const char *g_current_word = NULL;
static int g_total_sum = 0;
static int g_target_doc = -1;

static void sum_one_word(void *elementp);
static void add_post(void *pp);
static void dump_posting(void *pp);
static void dump_wordentry(void *elementp);
int index_dump_multi(hashtable_t *ht, FILE *out);
static int sumwords_doc(hashtable_t *ht, int docID);
static void free_posting(void *pp);
static void free_wordentry(void *elementp);
static void index_destroy_multi(hashtable_t *ht);
static void dump_posting_pretty(void *pp);
static void print_wordentry_pretty(void *elementp);





static int g_first;

int main(int argc, char *argv[]) {
    // ---- read docID from command line ----
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <id>\n", argv[0]);
        return 1;
    }
    int threshhold = atoi(argv[1]) + 1;
    int total_squared = 0;
    hashtable_t *ht = hopen(2048);
    for(int docID = 1; docID < threshhold; docID++) {
        // ---- load the webpage ----
        webpage_t *page = pageload(docID, "../crawler");
        if (!page) {
            fprintf(stderr, "failed to load page %d\n", docID);
            return 1;
        }

        // ---- create hashtable for index ----
        if (!ht) {
            fprintf(stderr, "hopen failed\n");
            webpage_delete(page);
            return 1;
        }

        // ---- build index for this page ----
        int pos = 0;
        char *raw = NULL;
        while ((pos = webpage_getNextWord(page, pos, &raw)) > 0) {
            char *norm = NormalizeWord(raw);
            if (norm) {
                index_update_doc(ht, norm, docID);
                free(norm);
            }
            free(raw);
        }

        // ---- print results ----
        printf("\n=== Inverted Index for Document %d ===\n", docID);
        happly(ht, print_wordentry_pretty);

        // ---- verify total word count ----
        int total = sumwords_doc(ht, docID);
        total_squared += total;
        printf("\nTOTAL word occurrences in doc%d: %d\n", docID, total);

        // ---- cleanup ----
        webpage_delete(page);
    } 
    index_save(ht, "step6_file");
    index_destroy_multi(ht);
    printf("Total total word count is: %d\n", total_squared);
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


static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static bool word_equals(void *elementp, const void *keyp) {
    const wordentry_t *we = (const wordentry_t *)elementp;
    return strcmp(we->word, (const char *)keyp) == 0;
}

static bool doc_equals(void *elementp, const void *keyp) {
    const posting_t *p = (const posting_t *)elementp;
    return p->docID == *(const int *)keyp;
}

static wordentry_t *wordentry_init(const char *word) {
    wordentry_t *we = malloc(sizeof(*we));
    if (!we) return NULL;
    we->word = xstrdup(word);
    if (!we->word) { free(we); return NULL; }
    we->plist = qopen();
    if (!we->plist) { free(we->word); free(we); return NULL; }
    return we;
}

static posting_t *posting_init(int docID) {
    posting_t *p = malloc(sizeof(*p));
    if (!p) return NULL;
    p->docID = docID;
    p->count = 1;
    return p;
}

static int index_update_doc(hashtable_t *ht, const char *norm_word, int docID) {
    if (!ht || !norm_word) return 1;

    int keylen = (int)strlen(norm_word);       // be consistent (no +1)
    wordentry_t *we = hsearch(ht, word_equals, norm_word, keylen);

    if (!we) {  // first time seeing this word
        we = wordentry_init(norm_word);
        if (!we) return 2;
        if (hput(ht, we, we->word, (int)strlen(we->word)) != 0) {
            // unlikely, but clean up on failure
            qclose(we->plist);
            free(we->word);
            free(we);
            return 3;
        }
    }

    // find posting for this doc
    posting_t *p = qsearch(we->plist, doc_equals, &docID);
    if (p) {
        p->count += 1;
        return 0;
    }
    // new doc for this word
    p = posting_init(docID);
    if (!p) return 4;
    qput(we->plist, p);
    return 0;
}

static void dump_wordentry(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    g_current_word = we->word;        // set global so dump_posting can use it
    qapply(we->plist, dump_posting);  // apply dump_posting to each posting
    g_current_word = NULL;            // clear afterward (not strictly needed)
}

int index_dump_multi(hashtable_t *ht, FILE *out) {
    if (!ht || !out) return 1;
    g_out_multi = out;
    happly(ht, dump_wordentry);
    g_out_multi = NULL;
    return 0;
}

static int sumwords_doc(hashtable_t *ht, int docID) {
    if (!ht) return 0;
    g_total_sum = 0;
    g_target_doc = docID;     // -1 means "sum over all docs"
    happly(ht, sum_one_word);
    return g_total_sum;
}

static void free_posting(void *pp) {
    free(pp);
}

static void free_wordentry(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    if (we->plist) { qapply(we->plist, free_posting); qclose(we->plist); }
    free(we->word);
    free(we);
}

static void index_destroy_multi(hashtable_t *ht) {
    if (!ht) return;
    happly(ht, free_wordentry);
    hclose(ht);
}

static void dump_posting_pretty(void *pp) {
    posting_t *p = (posting_t *)pp;
    if (!g_first) printf(", ");
    printf("(doc%d, %d)", p->docID, p->count);
    g_first = 0;
}

static void print_wordentry_pretty(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    printf("%s -> [", we->word);
    g_first = 1;
    qapply(we->plist, dump_posting_pretty);
    printf("]\n");
}

static void dump_posting(void *pp) {
    posting_t *p = (posting_t *)pp;
    fprintf(g_out_multi, "%s %d %d\n", g_current_word, p->docID, p->count);
}

static void add_post(void *pp) {
    posting_t *p = (posting_t *)pp;
    if (g_target_doc < 0 || p->docID == g_target_doc)
        g_total_sum += p->count;
}

static void sum_one_word(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    qapply(we->plist, add_post);
}

