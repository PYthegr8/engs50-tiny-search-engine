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
#include <inttypes.h>
#include <unistd.h>
#include <webpage.h>
#include "hash.h"
#include "queue.h"
#include "indexio.h"
#include "pageio.h"

int32_t index_save(hashtable_t *ht, char *indexnm);
int index_dump_multi(hashtable_t *ht, FILE *out);
hashtable_t *index_load(char *indexnm);




int32_t index_save(hashtable_t *ht, char *indexnm){
    char filepath[256];
    snprintf(filepath, sizeof filepath, "../indexer/%s", indexnm);
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        fprintf(stderr, "[Error: Could not open file %s]\n", filepath);
        return EXIT_FAILURE;
    }
    int rc = index_dump_multi(ht, file);
    fclose(file);
    return (rc == 0) ? 0 : EXIT_FAILURE;
}

/* Minimal index_load — concise and relies on well-formed input */
hashtable_t *index_load(const char *filename) {
    if (!filename) return NULL;
    FILE *in = fopen(filename, "r");
    if (!in) return NULL;

    hashtable_t *ht = hopen(4096);
    if (!ht) { fclose(in); return NULL; }

    char line[8192];           /* reasonably large for course data */
    char *saveptr;

    while (fgets(line, sizeof line, in)) {
        /* strip trailing newline (optional) */
        size_t L = strlen(line);
        if (L && line[L-1] == '\n') line[L-1] = '\0';

        /* first token is the word */
        char *tok = strtok_r(line, " \t\r\n", &saveptr);
        if (!tok) continue;

        /* get or create wordentry_t */
        wordentry_t *we = hsearch(ht, word_equals, tok, (int)strlen(tok) + 1);
        if (!we) {
            we = wordentry_init(tok);
            if (!we || hput(ht, we, we->word, (int)strlen(we->word) + 1) != 0) {
                if (we) free_wordentry(we);
                index_destroy_multi(ht);
                fclose(in);
                return NULL;
            }
        }

        /* remaining tokens are pairs: docID count */
        while (1) {
            char *t1 = strtok_r(NULL, " \t\r\n", &saveptr);
            if (!t1) break;
            char *t2 = strtok_r(NULL, " \t\r\n", &saveptr);
            if (!t2) { /* malformed (odd token) — bail */
                index_destroy_multi(ht);
                fclose(in);
                return NULL;
            }

            int docID = atoi(t1);      /* simple conversion (assumes valid numeric) */
            int count = atoi(t2);
            if (docID <= 0 || count <= 0) { /* basic sanity */
                index_destroy_multi(ht);
                fclose(in);
                return NULL;
            }

            posting_t *existing = qsearch(we->plist, doc_equals, &docID);
            if (existing) existing->count += count;
            else {
                posting_t *p = malloc(sizeof *p);
                if (!p) { index_destroy_multi(ht); fclose(in); return NULL; }
                p->docID = docID;
                p->count = count;
                qput(we->plist, p);
            }
        }
    }

    fclose(in);
    return ht;
}
