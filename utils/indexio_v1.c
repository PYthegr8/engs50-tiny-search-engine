/* 
 * indexio.c --- 
 * 
 * Author: Benjamin W. Rippy
 * Created: 10-30-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "queue.h"
#include "hash.h"
#include "indexio.h"

/* ----- data model expected in the hashtable ----- */
typedef struct { int docID; int count; } posting_t;
typedef struct { char *word; queue_t *plist; } wordentry_t;

/* ----- tiny helpers (C11-safe) ----- */
static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* ----- forward decls ----- */
static int  index_dump_multi(hashtable_t *ht, FILE *out);
static void dump_word_apply(void *elem);       /* matches: void (*)(void*) */
static void dump_posting_apply(void *ep);      /* matches: void (*)(void*) */
static int  parse_line_into_entry(const char *line, wordentry_t **out_we);

/* global used during dump happly/qapply */
static FILE *g_out = NULL;

/* =================================================== */
/*                    PUBLIC API                       */
/* =================================================== */

int32_t index_save(hashtable_t *ht, char *indexnm)
{
    if (!ht || !indexnm) return EXIT_FAILURE;

    char filepath[256];
    snprintf(filepath, sizeof filepath, "../indexer/%s", indexnm);

    FILE *file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "[Error: Could not open file %s]\n", filepath);
        return EXIT_FAILURE;
    }
    int rc = index_dump_multi(ht, file);
    fclose(file);
    return (rc == 0) ? 0 : EXIT_FAILURE;
}

hashtable_t *index_load(char *indexnm)
{
    if (!indexnm) return NULL;

    char filepath[256];
    snprintf(filepath, sizeof filepath, "../indexer/%s", indexnm);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[Error: Could not open index file %s]\n", filepath);
        return NULL;
    }

    hashtable_t *ht = hopen(1024);
    if (!ht) { fclose(fp); return NULL; }

    char line[8192];
    while (fgets(line, sizeof line, fp)) {
        /* strip trailing newline(s) */
        size_t n = strcspn(line, "\r\n");
        line[n] = '\0';
        if (n == 0) continue;

        wordentry_t *we = NULL;
        if (parse_line_into_entry(line, &we) != 0) {
            fprintf(stderr, "[Error: Malformed index line: %s]\n", line);
            fclose(fp);
            /* NOTE: if you need to free ht so far, do it here */
            return NULL;
        }
        if (hput(ht, we, we->word, (int)strlen(we->word)) != 0) {
            fprintf(stderr, "[Error: hput failed for word '%s']\n", we->word);
            fclose(fp);
            return NULL;
        }
    }

    fclose(fp);
    return ht;
}

/* =================================================== */
/*                   INTERNAL SAVE                     */
/* =================================================== */

static int index_dump_multi(hashtable_t *ht, FILE *out)
{
    if (!ht || !out) return -1;
    g_out = out;
    happly(ht, dump_word_apply);   /* callback: void (*)(void*) */
    g_out = NULL;
    return 0;
}

static void dump_word_apply(void *elem)
{
    wordentry_t *we = (wordentry_t *)elem;
    if (!we || !we->word || !we->plist) return;

    fprintf(g_out, "%s", we->word);
    qapply(we->plist, dump_posting_apply);   /* callback: void (*)(void*) */
    fputc('\n', g_out);
}

static void dump_posting_apply(void *ep)
{
    posting_t *p = (posting_t *)ep;
    if (!p) return;
    fprintf(g_out, " %d %d", p->docID, p->count);
}

/* =================================================== */
/*                   INTERNAL LOAD                     */
/* =================================================== */

static int parse_line_into_entry(const char *line, wordentry_t **out_we)
{
    *out_we = NULL;

    char *buf = xstrdup(line);
    if (!buf) return -1;

    char *tok = strtok(buf, " \t");
    if (!tok) { free(buf); return -1; }

    /* word must be lowercase letters per spec */
    const char *word = tok;
    for (const char *c = word; *c; c++) {
        if (!islower((unsigned char)*c)) { free(buf); return -1; }
    }

    wordentry_t *we = calloc(1, sizeof *we);
    if (!we) { free(buf); return -1; }

    we->word = xstrdup(word);
    if (!we->word) { free(we); free(buf); return -1; }

    queue_t *plist = qopen();
    if (!plist) { free(we->word); free(we); free(buf); return -1; }
    we->plist = plist;

    /* read pairs <docID> <count> ... */
    while (1) {
        char *docTok = strtok(NULL, " \t");
        if (!docTok) break;

        char *cntTok = strtok(NULL, " \t");
        if (!cntTok) { free(buf); return -1; }

        posting_t *post = malloc(sizeof *post);
        if (!post) { free(buf); return -1; }

        post->docID = (int)strtol(docTok, NULL, 10);
        post->count = (int)strtol(cntTok, NULL, 10);
        if (post->docID <= 0 || post->count <= 0) {
            free(post); free(buf); return -1;
        }
        qput(plist, post);
    }

    free(buf);
    *out_we = we;
    return 0;
}
