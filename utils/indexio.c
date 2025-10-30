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
//hashtable_t *index_load(char *indexnm);




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

