/*
Author: MergeConflict
November 2025
ENGS 50
Module 6: Querier
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "queue.h"
#include "hash.h"
#include "webpage.h"
#include "pageio.h"
#include "indexio.h"

static hashtable_t *g_inverted_index = NULL;
static char *g_current_word = NULL;
char *NormalizeWord(char *input);
void print_q_contents(void* elementp);
static bool word_equals(void *elementp, const void *keyp);
static bool doc_equals(void *elementp, const void *keyp);
static bool build_query_string(hashtable_t *ht, char *query_string, char **words, int count);
void get_doc_ids(hashtable_t *ht, char *word);
hashtable_t *init_inverted_index(hashtable_t *ht);

typedef struct {
    char *word;
    int frequency;
} wordcount_t;

typedef struct {
    int docID;
    int count;
} posting_t;

typedef struct {
    char *word;
    queue_t *plist;
} wordentry_t;

int main(void) {
    char line[1000];
    char *file_name = "mod6_step2_index";
    hashtable_t *ht = index_load(file_name);
    hashtable_t inverted_index = init_inverted_index(ht);

    printf("Please enter your input to the command line!\n");

    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("End of file signal entered!\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        char *token = strtok(line, " \t");
        if (!token) continue;

        int invalid = 0;
        char *words[100];
        int count = 0;

        while (token != NULL) {
            char *normalized = NormalizeWord(token);
            if (!normalized) {
                invalid = 1;
                break;
            }
            if (!strcmp(normalized, "and") || !strcmp(normalized, "or")) {
                token = strtok(NULL, " \t");
                continue;
            }
            words[count++] = normalized;
            token = strtok(NULL, " \t");
        }

        if (invalid) {
            printf("[invalid query]\n");
            for (int i = 0; i < count; i++)
                free(words[i]);
            continue;
        }

        char *query_string = malloc(10000 * sizeof(char));
        query_string[0] = '\0';

        if (build_query_string(ht, query_string, words, count)) {
            printf("%s\n", query_string);
            free(query_string);
        } else {
            printf("No matches found for your query!\n");
        }
    }

    return 0;
}

char *NormalizeWord(char *input) {
    int len = strlen(input);
    if (len < 1) return NULL;

    char *newWord = malloc(len + 1);
    if (!newWord) return NULL;

    for (int i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (!isalpha(c)) {
            free(newWord);
            return NULL;
        }
        newWord[i] = (char)tolower(c);
    }
    newWord[len] = '\0';
    return newWord;
}

static bool word_equals(void *elementp, const void *keyp) {
    const wordentry_t *we = (const wordentry_t *)elementp;
    return strcmp(we->word, (const char *)keyp) == 0;
}

static bool doc_equals(void *elementp, const void *keyp) {
    const posting_t *p = (const posting_t *)elementp;
    return p->docID == *(const int *)keyp;
}

static bool build_query_string(hashtable_t *ht, char *query_string, char **words, int count) {
    int lowest = INT_MAX;
    int word_match_count = 0;
    for (int i = 0; i < count; i++) {
        int keylen = (int)strlen(words[i]) + 1;
        int docID = 1;
        wordentry_t *we = hsearch(ht, word_equals, words[i], keylen);
        if (we) {
            posting_t *p = qsearch(we->plist, doc_equals, &docID);
            if (p) {
                word_match_count++;
                char buffer[20];
                int c = p->count;
                sprintf(buffer, "%d", c);
                lowest = fmin(lowest, c);
                strcat(query_string, words[i]);
                strcat(query_string, ":");
                strcat(query_string, buffer);
                strcat(query_string, " ");
            }
        }
        free(words[i]);
    }
    char lowest_buffer[20];
    sprintf(lowest_buffer, "%d", lowest);
    strcat(query_string, "-- ");
    strcat(query_string, lowest_buffer);
    return word_match_count ? true : false;
}

static void process_posting(void *postp) {
    posting_t *p = (posting_t *)postp;
    char key[20];
    sprintf(key, "%d", p->docID);

    queue_t *doc_words = hsearch(g_inverted_index, NULL, key, strlen(key) + 1);
    if (!doc_words) {
        doc_words = qopen();
        hput(g_inverted_index, doc_words, key, strlen(key) + 1);
    }

    wordcount_t *wc = malloc(sizeof(wordcount_t));
    wc->word = malloc(strlen(g_current_word) + 1);
    strcpy(wc->word, g_current_word);
    wc->frequency = p->count;
    qput(doc_words, wc);
}

static void process_word(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    g_current_word = we->word;
    qapply(we->plist, process_posting);
}

hashtable_t *init_inverted_index(hashtable_t *ht) {
    g_inverted_index = hopen(1000);
    happly(ht, process_word);
    return g_inverted_index;
}

