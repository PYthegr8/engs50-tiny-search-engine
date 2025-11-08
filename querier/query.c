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

typedef struct { int docID; int rank; } result_t;
typedef struct { int docID; int count; } posting_t;
typedef struct { char *word; queue_t *plist; } wordentry_t;

char *NormalizeWord(char *input);
static char *xstrdup(const char *s);
static char *get_url(const char *pagedir, int docID);
static result_t *get_postings_for_word(hashtable_t *ht, const char *word, int *outn);
static result_t *compute_and_sequence(hashtable_t *ht, char **words, int wcount, int *outn);
static result_t *or_merge(result_t *acc, int accn, result_t *seqr, int seqn, int *outn);
static int find_docidx(result_t *arr, int n, int docID);
static int compare_results(const void *a, const void *b);
static bool word_equals(void *elementp, const void *keyp);
static void free_posting(void *pp);
static void free_wordentry(void *ep);
static void index_destroy(hashtable_t *ht);

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s <pagedir> <indexfile> [-q]\n", argv[0]);
        return 1;
    }

    const char *pagedir = argv[1];
    const char *indexfile = argv[2];
    bool quiet = false;
    if (argc == 4) {
        if (strcmp(argv[3], "-q") != 0) {
            fprintf(stderr, "Usage: %s <pagedir> <indexfile> [-q]\n", argv[0]);
            return 1;
        }
        quiet = true;
    }

    DIR *d = opendir(pagedir);
    if (!d) {
        fprintf(stderr, "Error: pagedir '%s' does not exist\n", pagedir);
        return 1;
    }
    closedir(d);

    hashtable_t *ht = index_load((char *)indexfile);
    if (!ht) {
        fprintf(stderr, "Error: could not load index file '%s'\n", indexfile);
        return 1;
    }

    char rawline[2000];
    while (1) {
        if (!quiet) printf("> ");
        if (fgets(rawline, sizeof rawline, stdin) == NULL) break;
        rawline[strcspn(rawline, "\r\n")] = '\0';
        if (strlen(rawline) == 0) continue;

        char linecopy[2000];
        strncpy(linecopy, rawline, sizeof linecopy);
        linecopy[sizeof linecopy - 1] = '\0';

        char *tokens[256];
        int tcount = 0;
        char *tok = strtok(linecopy, " \t");
        while (tok && tcount < 256) {
            tokens[tcount++] = tok;
            tok = strtok(NULL, " \t");
        }
        if (tcount == 0) continue;

        char *normalized_tokens[256];
        int tok_type[256];
        bool invalid = false;
        for (int i = 0; i < 256; ++i) {
            normalized_tokens[i] = NULL;
            tok_type[i] = -1;
        }
        for (int i = 0; i < tcount; ++i) {
            char low[256];
            size_t L = strlen(tokens[i]);
            if (L >= sizeof low) { invalid = true; break; }
            for (size_t k = 0; k < L; ++k) low[k] = (char)tolower((unsigned char)tokens[i][k]);
            low[L] = '\0';
            if (strcmp(low, "and") == 0) { tok_type[i] = 1; normalized_tokens[i] = NULL; }
            else if (strcmp(low, "or") == 0) { tok_type[i] = 2; normalized_tokens[i] = NULL; }
            else {
                char *norm = NormalizeWord(tokens[i]);
                if (!norm) { invalid = true; break; }
                tok_type[i] = 0;
                normalized_tokens[i] = norm;
            }
        }

        if (invalid) {
            printf("[invalid query]\n");
            for (int i = 0; i < tcount; ++i)
                if (tok_type[i] == 0 && normalized_tokens[i]) free(normalized_tokens[i]);
            continue;
        }

        if (tok_type[0] != 0 || tok_type[tcount - 1] != 0) {
            printf("[invalid query]\n");
            for (int i = 0; i < tcount; ++i)
                if (tok_type[i] == 0 && normalized_tokens[i]) free(normalized_tokens[i]);
            continue;
        }

        bool syntax_ok = true;
        for (int i = 0; i < tcount; ++i) {
            if (tok_type[i] == 1 || tok_type[i] == 2) {
                if (i == 0 || i == tcount - 1) { syntax_ok = false; break; }
                if (tok_type[i-1] != 0 || tok_type[i+1] != 0) { syntax_ok = false; break; }
            }
        }
        if (!syntax_ok) {
            printf("[invalid query]\n");
            for (int i = 0; i < tcount; ++i)
                if (tok_type[i] == 0 && normalized_tokens[i]) free(normalized_tokens[i]);
            continue;
        }

        char **seq_words[128];
        int seq_counts[128];
        int seq_cap[128];
        int seqs = 0;
        seq_words[0] = NULL;
        seq_counts[0] = 0;
        seq_cap[0] = 0;

        for (int i = 0; i < tcount; ++i) {
            if (tok_type[i] == 2) {
                seqs++;
                seq_words[seqs] = NULL;
                seq_counts[seqs] = 0;
                seq_cap[seqs] = 0;
                continue;
            }
            if (tok_type[i] == 1) continue;
            if (seq_counts[seqs] == 0) {
                seq_cap[seqs] = 4;
                seq_words[seqs] = malloc(seq_cap[seqs] * sizeof(char*));
            }
            if (seq_counts[seqs] >= seq_cap[seqs]) {
                seq_cap[seqs] *= 2;
                seq_words[seqs] = realloc(seq_words[seqs], seq_cap[seqs] * sizeof(char*));
            }
            seq_words[seqs][seq_counts[seqs]++] = normalized_tokens[i];
            normalized_tokens[i] = NULL;
        }

        int total_sequences = seqs + 1;

        result_t *acc = NULL;
        int accn = 0;
        for (int s = 0; s < total_sequences; ++s) {
            int seqn = seq_counts[s];
            if (seqn == 0) continue;
            int seqr_n = 0;
            result_t *seqr = compute_and_sequence(ht, seq_words[s], seqn, &seqr_n);
            if (seqr == NULL || seqr_n == 0) {
                if (seqr) free(seqr);
                continue;
            }
            int newn = 0;
            result_t *newacc = or_merge(acc, accn, seqr, seqr_n, &newn);
            if (acc) free(acc);
            acc = newacc;
            accn = newn;
            free(seqr);
        }

        for (int s = 0; s < total_sequences; ++s) {
            if (seq_words[s]) {
                for (int j = 0; j < seq_counts[s]; ++j)
                    free(seq_words[s][j]);
                free(seq_words[s]);
            }
        }

        if (accn == 0 || acc == NULL) {
            printf("No matches found for your query!\n");
            if (acc) free(acc);
            for (int i = 0; i < tcount; ++i)
                if (tok_type[i] == 0 && normalized_tokens[i]) free(normalized_tokens[i]);
            continue;
        }

        qsort(acc, (size_t)accn, sizeof *acc, compare_results);

        for (int i = 0; i < accn; ++i) {
            char *url = get_url(pagedir, acc[i].docID);
            if (!url) url = xstrdup("URL_NOT_FOUND");
            printf("rank: %d: doc: %d : %s\n", acc[i].rank, acc[i].docID, url);
            free(url);
        }

        free(acc);
        for (int i = 0; i < tcount; ++i)
            if (tok_type[i] == 0 && normalized_tokens[i]) free(normalized_tokens[i]);
    }

    index_destroy(ht);
    return 0;
}

char *NormalizeWord(char *input)
{
    int len = (int)strlen(input);
    if (len < 1) return NULL;
    char *newWord = malloc((size_t)len + 1);
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
    wordentry_t *we = (wordentry_t *)elementp;
    const char *key = (const char *)keyp;
    return strcmp(we->word, key) == 0;
}

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
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

static result_t *get_postings_for_word(hashtable_t *ht, const char *word, int *outn)
{
    *outn = 0;
    int keylen = (int)strlen(word) + 1;
    wordentry_t *we = hsearch(ht, word_equals, (char *)word, keylen);
    if (!we) return NULL;

    int cap = 16;
    result_t *arr = malloc((size_t)cap * sizeof *arr);
    if (!arr) return NULL;

    queue_t *plist = we->plist;
    queue_t *tmp = qopen();
    void *p;
    while ((p = qget(plist)) != NULL) {
        posting_t *pp = (posting_t *)p;
        if (*outn >= cap) {
            cap *= 2;
            arr = realloc(arr, (size_t)cap * sizeof *arr);
            if (!arr) { qput(tmp, p); break; }
        }
        arr[*outn].docID = pp->docID;
        arr[*outn].rank = pp->count;
        (*outn)++;
        qput(tmp, p);
    }
    while ((p = qget(tmp)) != NULL) qput(plist, p);
    qclose(tmp);

    if (*outn == 0) { free(arr); return NULL; }
    return arr;
}

static result_t *compute_and_sequence(hashtable_t *ht, char **words, int wcount, int *outn)
{
    *outn = 0;
    if (wcount == 0) return NULL;
    int n0 = 0;
    result_t *cur = get_postings_for_word(ht, words[0], &n0);
    if (!cur || n0 == 0) { if (cur) free(cur); return NULL; }

    for (int i = 1; i < wcount; ++i) {
        int ni = 0;
        result_t *next = get_postings_for_word(ht, words[i], &ni);
        if (!next || ni == 0) { if (next) free(next); free(cur); return NULL; }

        int cap = n0 < ni ? n0 : ni;
        result_t *merged = malloc((size_t)(cap > 0 ? cap : 1) * sizeof *merged);
        int m = 0;
        for (int a = 0; a < n0; ++a) {
            int idx = find_docidx(next, ni, cur[a].docID);
            if (idx >= 0) {
                int r = cur[a].rank < next[idx].rank ? cur[a].rank : next[idx].rank;
                merged[m].docID = cur[a].docID;
                merged[m].rank = r;
                m++;
            }
        }
        free(cur);
        free(next);
        if (m == 0) { free(merged); return NULL; }
        cur = merged;
        n0 = m;
    }
    *outn = n0;
    return cur;
}

static result_t *or_merge(result_t *acc, int accn, result_t *seqr, int seqn, int *outn)
{
    if ((!acc || accn == 0) && (!seqr || seqn == 0)) { *outn = 0; return NULL; }

    int cap = (accn > 0 ? accn : 0) + (seqn > 0 ? seqn : 0);
    result_t *out = malloc((size_t)(cap > 0 ? cap : 1) * sizeof *out);
    int n = 0;

    if (acc && accn > 0) {
        for (int i = 0; i < accn; ++i) out[n++] = acc[i];
    }

    if (seqr && seqn > 0) {
        for (int j = 0; j < seqn; ++j) {
            int idx = find_docidx(out, n, seqr[j].docID);
            if (idx >= 0) out[idx].rank += seqr[j].rank;
            else out[n++] = seqr[j];
        }
    }

    *outn = n;
    return out;
}

static int find_docidx(result_t *arr, int n, int docID)
{
    if (!arr || n == 0) return -1;
    for (int i = 0; i < n; ++i) if (arr[i].docID == docID) return i;
    return -1;
}

static int compare_results(const void *a, const void *b)
{
    const result_t *ra = (const result_t *)a;
    const result_t *rb = (const result_t *)b;
    if (rb->rank != ra->rank) return rb->rank - ra->rank;
    return ra->docID - rb->docID;
}

static void free_posting(void *pp)
{
    free(pp);
}

static void free_wordentry(void *ep)
{
    wordentry_t *we = (wordentry_t *)ep;
    if (we->plist) {
        qapply(we->plist, free_posting);
        qclose(we->plist);
    }
    free(we->word);
    free(we);
}

static void index_destroy(hashtable_t *ht)
{
    if (!ht) return;
    happly(ht, free_wordentry);
    hclose(ht);
}
