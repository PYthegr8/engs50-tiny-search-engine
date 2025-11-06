#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include "queue.h"
#include "hash.h"
#include "webpage.h"
#include "pageio.h"
#include "indexio.h"

char *NormalizeWord(char *input);
static bool word_equals(void *elementp, const void *keyp);
static bool doc_equals(void *elementp, const void *keyp);
static char *get_url(const char *pagedir, int docID);
static bool process_and_query(hashtable_t *ht, const char *pagedir, char **words, int count);
static char *xstrdup(const char *s);

typedef struct { int docID; int count; } posting_t;
typedef struct { char *word; queue_t *plist; } wordentry_t;
typedef struct { int docID; int rank; } result_t;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pagedir> <indexfile>\n", argv[0]);
        return 1;
    }
    const char *pagedir = argv[1];
    char *indexfile = argv[2];
    DIR *d = opendir(pagedir);
    if (!d) {
        fprintf(stderr, "Error: pagedir '%s' does not exist\n", pagedir);
        return 1;
    }
    closedir(d);
    hashtable_t *ht = index_load(indexfile);
    if (!ht) {
        fprintf(stderr, "Error: could not load index file '%s'\n", indexfile);
        return 1;
    }
    char line[1000];
    while (1) {
        printf("> ");
        if (fgets(line, sizeof line, stdin) == NULL) break;
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        char *tok = strtok(line, " \t");
        if (!tok) continue;
        int invalid = 0;
        char *words[100];
        int wcount = 0;
        while (tok != NULL) {
            char *norm = NormalizeWord(tok);
            if (!norm) { invalid = 1; break; }
            if (!strcmp(norm, "and") || !strcmp(norm, "or")) { free(norm); tok = strtok(NULL, " \t"); continue; }
            words[wcount++] = norm;
            tok = strtok(NULL, " \t");
        }
        if (invalid) {
            printf("[invalid query]\n");
            for (int i = 0; i < wcount; ++i) free(words[i]);
            continue;
        }
        if (wcount == 0) continue;
        bool ok = process_and_query(ht, pagedir, words, wcount);
        for (int i = 0; i < wcount; ++i) free(words[i]);
        if (!ok) printf("No matches found for your query!\n");
    }
    hclose(ht);
    return 0;
}

char *NormalizeWord(char *input)
{
    int len = strlen(input);
    if (len < 1) return NULL;
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

static bool word_equals(void *elementp, const void *keyp)
{
    const wordentry_t *we = (const wordentry_t *)elementp;
    return strcmp(we->word, (const char *)keyp) == 0;
}

static bool doc_equals(void *elementp, const void *keyp)
{
    const posting_t *p = (const posting_t *)elementp;
    return p->docID == *(const int *)keyp;
}

static char *get_url(const char *pagedir, int docID)
{
    char path[512];
    snprintf(path, sizeof path, "%s/%d", pagedir, docID);
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    char buf[4096];
    if (!fgets(buf, sizeof buf, f)) { fclose(f); return NULL; }
    fclose(f);
    buf[strcspn(buf, "\r\n")] = '\0';
    return xstrdup(buf);
}

static int cmp_results(const void *a, const void *b)
{
    const result_t *ra = (const result_t *)a;
    const result_t *rb = (const result_t *)b;
    if (rb->rank != ra->rank) return rb->rank - ra->rank;
    return ra->docID - rb->docID;
}

static bool process_and_query(hashtable_t *ht, const char *pagedir, char **words, int count)
{
    int keylen0 = (int)strlen(words[0]) + 1;
    wordentry_t *we0 = hsearch(ht, word_equals, words[0], keylen0);
    if (!we0) return false;
    int cap = 32;
    int n = 0;
    result_t *results = malloc(cap * sizeof *results);
    queue_t *plist0 = we0->plist;
    queue_t *temp = qopen();
    void *postp;
    while ((postp = qget(plist0)) != NULL) {
        posting_t *p = (posting_t *)postp;
        int doc = p->docID;
        int rank = p->count;
        bool ok = true;
        for (int i = 1; i < count; ++i) {
            int keyleni = (int)strlen(words[i]) + 1;
            wordentry_t *wei = hsearch(ht, word_equals, words[i], keyleni);
            if (!wei) { ok = false; break; }
            posting_t *pi = qsearch(wei->plist, doc_equals, &doc);
            if (!pi) { ok = false; break; }
            if (pi->count < rank) rank = pi->count;
        }
        if (ok) {
            if (n >= cap) { cap *= 2; results = realloc(results, cap * sizeof *results); }
            results[n].docID = doc;
            results[n].rank = rank;
            n++;
        }
        qput(temp, p);
    }
    while ((postp = qget(temp)) != NULL) qput(plist0, postp);
    qclose(temp);
    if (n == 0) { free(results); return false; }
    qsort(results, n, sizeof *results, cmp_results);
    for (int i = 0; i < n; ++i) {
        int docID = results[i].docID;
        int rank = results[i].rank;
        char *url = get_url(pagedir, docID);
        if (!url) url = xstrdup("URL_NOT_FOUND");
        printf("rank: %d: doc: %d : %s\n", rank, docID, url);
        free(url);
    }
    free(results);
    return true;
}

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}
