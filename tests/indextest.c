/* 
 * indextest.c --- 
 * 
 * Author: Benjamin W. Rippy
 * Created: 10-30-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash.h"
#include "queue.h"
#include "indexio.h"

/* Match utils/indexio.c data model */
typedef struct { int docID; int count; } posting_t;
typedef struct { char *word; queue_t *plist; } wordentry_t;

/* ---------- small utils ---------- */
static char *xstrdup(const char *s){
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* dynamic string vector */
typedef struct { char **a; size_t n, cap; } strvec_t;
static void sv_init(strvec_t *v){ v->a=NULL; v->n=0; v->cap=0; }
static void sv_push(strvec_t *v, char *s){
    if (v->n == v->cap){
        size_t nc = v->cap ? v->cap*2 : 64;
        char **na = realloc(v->a, nc * sizeof *na);
        if (!na) { free(s); return; }
        v->a = na; v->cap = nc;
    }
    v->a[v->n++] = s;
}
static void sv_free(strvec_t *v){
    for(size_t i=0;i<v->n;i++) free(v->a[i]);
    free(v->a);
    v->a=NULL; v->n=v->cap=0;
}

/* qsort helpers */
static int cmp_strp(const void *pa, const void *pb){
    const char *a = *(const char * const *)pa;
    const char *b = *(const char * const *)pb;
    return strcmp(a,b);
}
static int cmp_posting_docid(const void *pa, const void *pb){
    const posting_t *x = (const posting_t*)pa;
    const posting_t *y = (const posting_t*)pb;
    return (x->docID > y->docID) - (x->docID < y->docID);
}

/* ---------- build a small in-memory index ---------- */
static wordentry_t *make_wordentry(const char *w, const int *pairs, int npairs){
    wordentry_t *we = calloc(1, sizeof *we);
    if (!we) return NULL;
    we->word = xstrdup(w);
    if (!we->word){ free(we); return NULL; }

    we->plist = qopen();
    if (!we->plist){ free(we->word); free(we); return NULL; }

    for (int i = 0; i < npairs; i += 2){
        posting_t *p = malloc(sizeof *p);
        if (!p) return NULL;
        p->docID = pairs[i];
        p->count = pairs[i+1];
        qput(we->plist, p);
    }
    return we;
}
static int put_wordentry(hashtable_t *ht, wordentry_t *we){
    return hput(ht, we, we->word, (int)strlen(we->word));
}

/* ---------- canonicalize one index file into sorted lines ---------- */
/* We parse each line, sort its posting pairs by docID, re-emit a normalized line,
   collect all normalized lines into a vector, sort the vector lexicographically. */
static int canonicalize_file(const char *path, strvec_t *out){
    sv_init(out);
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    char buf[8192];
    while (fgets(buf, sizeof buf, fp)){
        /* trim newline(s) */
        size_t n = strcspn(buf, "\r\n");
        buf[n] = '\0';
        if (n == 0) continue;

        /* tokenize */
        char *tok = strtok(buf, " \t");
        if (!tok){ fclose(fp); sv_free(out); return 0; }

        /* word must be lowercase letters per spec; we accept if already lowercase */
        const char *word = tok;
        for (const char *c = word; *c; c++){
            if (!islower((unsigned char)*c)){ fclose(fp); sv_free(out); return 0; }
        }

        /* read postings into a temp array */
        posting_t *arr = NULL;
        size_t m = 0, cap = 0;

        while (1){
            char *d = strtok(NULL, " \t");
            if (!d) break;
            char *ct = strtok(NULL, " \t");
            if (!ct){ free(arr); fclose(fp); sv_free(out); return 0; }

            int doc = (int)strtol(d, NULL, 10);
            int cnt = (int)strtol(ct, NULL, 10);
            if (doc <= 0 || cnt <= 0){ free(arr); fclose(fp); sv_free(out); return 0; }

            if (m == cap){
                size_t nc = cap ? cap*2 : 8;
                posting_t *na = realloc(arr, nc * sizeof *na);
                if (!na){ free(arr); fclose(fp); sv_free(out); return 0; }
                arr = na; cap = nc;
            }
            arr[m].docID = doc;
            arr[m].count = cnt;
            m++;
        }

        /* sort postings by docID for canonical form */
        if (m > 1) qsort(arr, m, sizeof *arr, cmp_posting_docid);

        /* build normalized line: "<word> d1 c1 d2 c2 ...\n" */
        size_t L = strlen(word) + 2 + m * 22; /* rough upper bound */
        char *line = malloc(L);
        if (!line){ free(arr); fclose(fp); sv_free(out); return 0; }
        size_t off = 0;
        off += (size_t)snprintf(line+off, L-off, "%s", word);
        for (size_t i = 0; i < m; i++){
            off += (size_t)snprintf(line+off, L-off, " %d %d", arr[i].docID, arr[i].count);
        }
        /* no trailing spaces, no newline */

        sv_push(out, line);
        free(arr);
    }

    fclose(fp);

    /* sort all normalized lines lexicographically */
    if (out->n > 1) qsort(out->a, out->n, sizeof *out->a, cmp_strp);
    return 1;
}

static int files_equal_canonical(const char *p1, const char *p2){
    strvec_t a, b;
    if (!canonicalize_file(p1, &a)) return 0;
    if (!canonicalize_file(p2, &b)){ sv_free(&a); return 0; }
    int ok = 1;
    if (a.n != b.n) ok = 0;
    else {
        for (size_t i = 0; i < a.n; i++){
            if (strcmp(a.a[i], b.a[i]) != 0){ ok = 0; break; }
        }
    }
    sv_free(&a); sv_free(&b);
    return ok;
}

/* ---------- main test ---------- */
int main(void){
    /* 1) Build small index */
    hashtable_t *ht = hopen(64);

    int apple_pairs[] = {3,7, 8,2, 9,4};
    int cat_pairs[]   = {2,1};
    int that_pairs[]  = {1,3, 2,5};

    if (put_wordentry(ht, make_wordentry("that",  that_pairs, 4)) != 0){ fprintf(stderr,"FAIL put 'that'\n"); return 1; }
    if (put_wordentry(ht, make_wordentry("cat",   cat_pairs,  2)) != 0){ fprintf(stderr,"FAIL put 'cat'\n");  return 1; }
    if (put_wordentry(ht, make_wordentry("apple", apple_pairs,6)) != 0){ fprintf(stderr,"FAIL put 'apple'\n");return 1; }

    const char *f1 = "io_round1.idx";
    const char *f2 = "io_round2.idx";
    char path1[256], path2[256];
    snprintf(path1, sizeof path1, "../indexer/%s", f1);
    snprintf(path2, sizeof path2, "../indexer/%s", f2);

    /* 2) Save -> Load -> Save */
    if (index_save(ht, (char*)f1) != 0){
        fprintf(stderr, "FAIL: index_save round1\n"); return 1;
    }
    hashtable_t *ht2 = index_load((char*)f1);
    if (!ht2){
        fprintf(stderr, "FAIL: index_load round1\n"); return 1;
    }
    if (index_save(ht2, (char*)f2) != 0){
        fprintf(stderr, "FAIL: index_save round2\n"); return 1;
    }

    /* 3) Canonical, order-insensitive compare */
    if (!files_equal_canonical(path1, path2)){
        fprintf(stderr, "FAIL: round-trip mismatch between %s and %s\n", path1, path2);
        return 1;
    }
    printf("PASS: round-trip canonical match (%s == %s)\n", path1, path2);

    /* 4) Empty index saves empty file */
    {
        hashtable_t *empty = hopen(8);
        const char *fempty = "io_empty.idx";
        char pathempty[256];
        snprintf(pathempty, sizeof pathempty, "../indexer/%s", fempty);

        if (index_save(empty, (char*)fempty) != 0){
            fprintf(stderr, "FAIL: index_save(empty)\n"); return 1;
        }
        FILE *fe = fopen(pathempty, "rb");
        if (!fe){ fprintf(stderr, "FAIL: cannot open %s\n", pathempty); return 1; }
        int c = fgetc(fe); fclose(fe);
        if (c != EOF){
            fprintf(stderr, "FAIL: empty index produced non-empty file\n");
            return 1;
        }
        printf("PASS: empty index saved as empty file (%s)\n", pathempty);
    }

    /* 5) Malformed file rejected (uppercase word + odd tokens) */
    {
        const char *badname = "io_bad.idx";
        char badpath[256];
        snprintf(badpath, sizeof badpath, "../indexer/%s", badname);
        FILE *fb = fopen(badpath, "w");
        if (!fb){ fprintf(stderr, "FAIL: cannot create %s\n", badpath); return 1; }
        fprintf(fb, "BaD 1 2 3\n");
        fclose(fb);

        hashtable_t *bad = index_load((char*)badname);
        if (bad){
            fprintf(stderr, "FAIL: malformed file unexpectedly loaded\n");
            return 1;
        }
        printf("PASS: malformed file rejected by loader (%s)\n", badpath);
    }

    printf("ALL TESTS PASSED\n");
    return 0;
}
