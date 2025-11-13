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
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include "queue.h"
#include "hash.h"
#include "webpage.h"
#include "pageio.h"
#include "indexio.h"

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

typedef struct {
    int start;
    int end;
    char *pagedir;
    hashtable_t *ht;
    char *indexnm;
} index_args_t;

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

char *NormalizeWord(char *input) {
    int len = strlen(input);
    if (len < 3) return NULL;
    char *newWord = malloc(len + 1);
    if (!newWord) return NULL;
    for (int i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (!isalpha(c)) { free(newWord); return NULL; }
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

static wordentry_t *wordentry_init(const char *word) {
    wordentry_t *we = malloc(sizeof *we);
    if (!we) return NULL;
    we->word = xstrdup(word);
    if (!we->word) { free(we); return NULL; }
    we->plist = qopen();
    if (!we->plist) { free(we->word); free(we); return NULL; }
    return we;
}

static posting_t *posting_init(int docID) {
    posting_t *p = malloc(sizeof *p);
    if (!p) return NULL;
    p->docID = docID;
    p->count = 1;
    return p;
}

static int index_update_doc(hashtable_t *ht, const char *norm_word, int docID) {
    if (!ht || !norm_word) return 1;
    int keylen = (int)strlen(norm_word) + 1;
    wordentry_t *we = hsearch(ht, word_equals, norm_word, keylen);
    if (!we) {
        we = wordentry_init(norm_word);
        if (!we) return 2;
        if (hput(ht, we, we->word, (int)strlen(we->word) + 1) != 0) {
            qclose(we->plist);
            free(we->word);
            free(we);
            return 3;
        }
    }
    posting_t *p = qsearch(we->plist, doc_equals, &docID);
    if (p) {
        p->count += 1;
        return 0;
    }
    p = posting_init(docID);
    if (!p) return 4;
    qput(we->plist, p);
    return 0;
}

static void free_posting(void *pp) {
    free(pp);
}

static void free_wordentry(void *elementp) {
    wordentry_t *we = (wordentry_t *)elementp;
    if (we->plist) {
        qapply(we->plist, free_posting);
        qclose(we->plist);
    }
    free(we->word);
    free(we);
}

static void index_destroy_multi(hashtable_t *ht) {
    if (!ht) return;
    happly(ht, free_wordentry);
    hclose(ht);
}

int count_files(char *pagedir) {
    int count = 0;
    for (int docID = 1;; docID++) {
        webpage_t *page = pageload(docID, (char *)pagedir);
        if (!page) break;
        count++;
    }
    return count;
}

void index(int start, int end, char *pagedir, hashtable_t *ht, char *indexnm) {
    for (int docID = start; docID <= end; docID++) {
        webpage_t *page = pageload(docID, (char *)pagedir);
        if (!page) break;
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
        webpage_delete(page);
    }
    if (index_save(ht, indexnm) != 0) {
        fprintf(stderr, "Error: index_save failed for '%s'\n", indexnm);
        index_destroy_multi(ht);
        return;
    }
    return;
}

void *index_thread(void *arg) {
    index_args_t *args = (index_args_t *)arg;
    index(args->start, args->end, args->pagedir, args->ht, args->indexnm);
    free(arg);
    return NULL;
}

/*
for (docID = 1;;docID++) {
    webpage_t *page = pageload(docID, (char *)pagedir);
    if (!page) break;
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
    webpage_delete(page);
}
*/

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <pagedir> <indexnm> <num_threads>\n", argv[0]);
        return 1;
    }
    const char *pagedir = argv[1];
    const char *indexnm = argv[2];
    int num_threads = atoi(argv[3]);

    DIR *d = opendir(pagedir);
    if (!d) {
        fprintf(stderr, "Error: pagedir '%s' does not exist\n", pagedir);
        return 1;
    }
    closedir(d);

    hashtable_t *ht = hopen(2048);
    if (!ht) {
        fprintf(stderr, "Error: hopen failed\n");
        return 1;
    }

    int start = 1;
    int files_per_thread = count_files((char *)pagedir);
    if (!files_per_thread) {
        fprintf(stderr, "[Error: no files found in the directory]\n");
        return 1;
    }
    files_per_thread /= num_threads;

    int thread_num = num_threads;
    while (thread_num--) {
        pthread_t thread_num;
        int end = start + files_per_thread;
        start = end;
        index_args_t args = { .start = start, .end = end, .pagedir = (char *)pagedir, .ht = ht, .indexnm = (char *)indexnm };
        pthread_create(&thread_num, NULL, index_thread, &args);
    }

    for (int i = 1; i <= num_threads; i++) {
        pthread_join(i, NULL);
    }

    index_destroy_multi(ht);
    return 0;
}

