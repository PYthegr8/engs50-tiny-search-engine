/* 
 * indexio.c --- 
 * 
 * Author: engs50 Team MergeConflict
 * Created: 10-29-2025
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

typedef struct { int docID; int count; } posting_t;
typedef struct { char *word; queue_t *plist; } wordentry_t;

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static int index_dump_multi(hashtable_t *ht, FILE *out);
static void dump_word_apply(void *elem);
static void dump_posting_apply(void *ep);
static int parse_line_into_entry(const char *line, wordentry_t **out_we);

static FILE *g_out = NULL;

int32_t index_save(hashtable_t *ht, const char *indexnm)
{
    if (!ht || !indexnm) return EXIT_FAILURE;
    FILE *file = fopen(indexnm, "w");
    if (!file) { perror("fopen"); return EXIT_FAILURE; }
    int rc = index_dump_multi(ht, file);
    fclose(file);
    return (rc == 0) ? 0 : EXIT_FAILURE;
}

hashtable_t *index_load(const char *indexnm)
{
    if (!indexnm) return NULL;
    FILE *fp = fopen(indexnm, "r");
    if (!fp) return NULL;

    hashtable_t *ht = hopen(1024);
    if (!ht) { fclose(fp); return NULL; }

    char line[8192];
    while (fgets(line, sizeof line, fp)) {
        size_t n = strcspn(line, "\r\n");
        line[n] = '\0';
        if (n == 0) continue;

        wordentry_t *we = NULL;
        if (parse_line_into_entry(line, &we) != 0) {
            fclose(fp);
            return NULL;
        }
        if (hput(ht, we, we->word, (int)strlen(we->word) + 1) != 0) {
            qapply(we->plist, free);
            qclose(we->plist);
            free(we->word);
            free(we);
            fclose(fp);
            return NULL;
        }
    }

    fclose(fp);
    return ht;
}

static int index_dump_multi(hashtable_t *ht, FILE *out)
{
    if (!ht || !out) return -1;
    g_out = out;
    happly(ht, dump_word_apply);
    g_out = NULL;
    return 0;
}

static void dump_word_apply(void *elem)
{
    wordentry_t *we = (wordentry_t *)elem;
    if (!we || !we->word || !we->plist) return;
    fprintf(g_out, "%s", we->word);
    qapply(we->plist, dump_posting_apply);
    fputc('\n', g_out);
}

static void dump_posting_apply(void *ep)
{
    posting_t *p = (posting_t *)ep;
    if (!p) return;
    fprintf(g_out, " %d %d", p->docID, p->count);
}

static int parse_line_into_entry(const char *line, wordentry_t **out_we)
{
    *out_we = NULL;
    char *buf = xstrdup(line);
    if (!buf) return -1;

    char *tok = strtok(buf, " \t");
    if (!tok) { free(buf); return -1; }

    for (char *c = tok; *c; ++c) {
        if (!islower((unsigned char)*c)) { free(buf); return -1; }
    }

    wordentry_t *we = calloc(1, sizeof *we);
    if (!we) { free(buf); return -1; }

    we->word = xstrdup(tok);
    if (!we->word) { free(we); free(buf); return -1; }

    we->plist = qopen();
    if (!we->plist) { free(we->word); free(we); free(buf); return -1; }

    while (1) {
        char *docTok = strtok(NULL, " \t");
        if (!docTok) break;
        char *cntTok = strtok(NULL, " \t");
        if (!cntTok) {
            qapply(we->plist, free);
            qclose(we->plist);
            free(we->word);
            free(we);
            free(buf);
            return -1;
        }

        posting_t *post = malloc(sizeof *post);
        if (!post) {
            qapply(we->plist, free);
            qclose(we->plist);
            free(we->word);
            free(we);
            free(buf);
            return -1;
        }

        post->docID = atoi(docTok);
        post->count = atoi(cntTok);
        if (post->docID <= 0 || post->count <= 0) {
            free(post);
            qapply(we->plist, free);
            qclose(we->plist);
            free(we->word);
            free(we);
            free(buf);
            return -1;
        }
        qput(we->plist, post);
    }

    free(buf);
    *out_we = we;
    return 0;
}
