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
static bool word_equals(void *elementp, const void *keyp);
static bool doc_equals(void *elementp, const void *keyp);
static char *get_url(const char *pagedir, int docID);
static char *xstrdup(const char *s);
static result_t *get_postings_for_word(hashtable_t *ht, const char *word, int *outn);
static result_t *set_and(result_t *a, int na, result_t *b, int nb, int *routn);
static result_t *set_or(result_t *a, int na, result_t *b, int nb, int *routn);
static void free_results(result_t *r);
static char **tokenize_expr(char *s, int *tokc);
static char **infix_to_postfix(char **toks, int tc, int *pcount);
static bool process_query_expression(hashtable_t *ht, const char *pagedir, char *input);
static int compare_results(const void *a, const void *b);
static void free_token_list(char **toks, int n);

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
        bool ok = process_query_expression(ht, pagedir, line);
        if (!ok) printf("No matches found for your query!\n");
    }
    hclose(ht);
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

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static result_t *get_postings_for_word(hashtable_t *ht, const char *word, int *outn)
{
    *outn = 0;
    int keylen = (int)strlen(word) + 1;
    wordentry_t *we = hsearch(ht, word_equals, (char *)word, keylen);
    if (!we) return NULL;
    int cap = 32;
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

static result_t *set_and(result_t *a, int na, result_t *b, int nb, int *routn)
{
    *routn = 0;
    if (!a || !b || na == 0 || nb == 0) return NULL;
    int cap = na < nb ? na : nb;
    result_t *out = malloc((size_t)(cap > 0 ? cap : 1) * sizeof *out);
    int n = 0;
    for (int i = 0; i < na; ++i) {
        for (int j = 0; j < nb; ++j) {
            if (a[i].docID == b[j].docID) {
                int rank = a[i].rank < b[j].rank ? a[i].rank : b[j].rank;
                out[n].docID = a[i].docID;
                out[n].rank = rank;
                n++;
                break;
            }
        }
    }
    if (n == 0) { free(out); *routn = 0; return NULL; }
    *routn = n;
    return out;
}

static result_t *set_or(result_t *a, int na, result_t *b, int nb, int *routn)
{
    *routn = 0;
    if ((!a || na == 0) && (!b || nb == 0)) return NULL;

    int cap = (na > 0 ? na : 0) + (nb > 0 ? nb : 0);
    result_t *out = malloc((size_t)(cap > 0 ? cap : 1) * sizeof *out);
    int n = 0;

    if (a && na > 0) {
        for (int i = 0; i < na; ++i) {
            out[n++] = a[i]; // copy a
        }
    }

    if (b && nb > 0) {
        for (int j = 0; j < nb; ++j) {
            int found = 0;
            for (int i = 0; i < n; ++i) {
                if (out[i].docID == b[j].docID) {
                    out[i].rank += b[j].rank; // SUM counts when present in both
                    found = 1;
                    break;
                }
            }
            if (!found) {
                out[n++] = b[j]; // append unique from b
            }
        }
    }

    *routn = n;
    return out;
}

static void free_results(result_t *r) { free(r); }

static char **tokenize_expr(char *s, int *tokc)
{
    int cap = 32;
    char **toks = malloc((size_t)cap * sizeof *toks);
    int n = 0;
    int i = 0;
    while (s[i]) {
        while (isspace((unsigned char)s[i])) ++i;
        if (!s[i]) break;
        if (s[i] == '(' || s[i] == ')') {
            char tmp[2] = { s[i], '\0' };
            if (n >= cap) { cap *= 2; toks = realloc(toks, (size_t)cap * sizeof *toks); }
            toks[n++] = xstrdup(tmp);
            ++i;
            continue;
        }
        int j = i;
        while (s[j] && !isspace((unsigned char)s[j]) && s[j] != '(' && s[j] != ')') ++j;
        int len = j - i;
        char *tok = malloc((size_t)len + 1);
        memcpy(tok, &s[i], (size_t)len);
        tok[len] = '\0';
        if (n >= cap) { cap *= 2; toks = realloc(toks, (size_t)cap * sizeof *toks); }
        toks[n++] = tok;
        i = j;
    }
    *tokc = n;
    return toks;
}

static char **infix_to_postfix(char **toks, int tc, int *pcount)
{
    char **out = malloc((size_t)tc * sizeof *out);
    char **op = malloc((size_t)tc * sizeof *op);
    int opn = 0;
    int outn = 0;
    for (int i = 0; i < tc; ++i) {
        char *t = toks[i];
        char lowbuf[256];
        int L = (int)strlen(t);
        for (int k = 0; k < L && k < (int)sizeof lowbuf - 1; ++k) lowbuf[k] = (char)tolower((unsigned char)t[k]);
        lowbuf[L] = '\0';
        if (!strcmp(lowbuf, "and") || !strcmp(lowbuf, "or")) {
            int prec = !strcmp(lowbuf, "and") ? 2 : 1;
            while (opn > 0) {
                char *top = op[opn-1];
                char toplowbuf[256];
                int TL = (int)strlen(top);
                for (int k = 0; k < TL && k < (int)sizeof toplowbuf - 1; ++k) toplowbuf[k] = (char)tolower((unsigned char)top[k]);
                toplowbuf[TL] = '\0';
                if (!strcmp(toplowbuf, "(")) break;
                int tprec = !strcmp(toplowbuf, "and") ? 2 : 1;
                if (tprec >= prec) { out[outn++] = op[--opn]; } else break;
            }
            op[opn++] = toks[i];
        } else if (!strcmp(lowbuf, "(")) {
            op[opn++] = toks[i];
        } else if (!strcmp(lowbuf, ")")) {
            while (opn > 0 && strcmp(op[opn-1], "(") != 0) out[outn++] = op[--opn];
            if (opn > 0 && !strcmp(op[opn-1], "(")) opn--;
        } else {
            out[outn++] = toks[i];
        }
    }
    while (opn > 0) out[outn++] = op[--opn];
    free(op);
    *pcount = outn;
    return out;
}

static void free_token_list(char **toks, int n)
{
    if (!toks) return;
    for (int i = 0; i < n; ++i) free(toks[i]);
    free(toks);
}

static int compare_results(const void *a, const void *b)
{
    const result_t *ra = (const result_t *)a;
    const result_t *rb = (const result_t *)b;
    if (rb->rank != ra->rank) return rb->rank - ra->rank;
    return ra->docID - rb->docID;
}

static bool process_query_expression(hashtable_t *ht, const char *pagedir, char *input)
{
    int tc;
    char **toks = tokenize_expr(input, &tc);
    if (tc == 0) { free(toks); return false; }
    for (int i = 0; i < tc; ++i) {
        for (char *p = toks[i]; *p; ++p) *p = (char)tolower((unsigned char)*p);
    }
    int pc;
    char **post = infix_to_postfix(toks, tc, &pc);
    typedef struct { result_t *arr; int len; } stkent;
    stkent *stack = malloc((size_t)(pc + 2) * sizeof *stack);
    int sn = 0;
    bool invalid = false;
    for (int i = 0; i < pc; ++i) {
        char *t = post[i];
        if (!strcmp(t, "and") || !strcmp(t, "or")) {
            if (sn < 2) { invalid = true; break; }
            stkent b = stack[--sn];
            stkent a = stack[--sn];
            stkent res;
            if (!strcmp(t, "and")) {
                res.arr = set_and(a.arr, a.len, b.arr, b.len, &res.len);
            } else {
                res.arr = set_or(a.arr, a.len, b.arr, b.len, &res.len);
            }
            free_results(a.arr);
            free_results(b.arr);
            stack[sn++] = res;
        } else if (!strcmp(t, "(") || !strcmp(t, ")")) {
            invalid = true;
            break;
        } else {
            int n;
            result_t *plist = get_postings_for_word(ht, t, &n);
            stkent s; s.arr = plist; s.len = n;
            stack[sn++] = s;
        }
    }
    free_token_list(toks, tc);
    free(post);
    if (invalid || sn != 1) {
        for (int k = 0; k < sn; ++k) free_results(stack[k].arr);
        free(stack);
        return false;
    }
    stkent final = stack[0];
    free(stack);
    if (final.len == 0) { free_results(final.arr); return false; }
    qsort(final.arr, final.len, sizeof *final.arr, compare_results);
    for (int i = 0; i < final.len; ++i) {
        char *url = get_url(pagedir, final.arr[i].docID);
        if (!url) url = xstrdup("URL_NOT_FOUND");
        printf("rank: %d: doc: %d : %s\n", final.arr[i].rank, final.arr[i].docID, url);
        free(url);
    }
    free_results(final.arr);
    return true;
}
