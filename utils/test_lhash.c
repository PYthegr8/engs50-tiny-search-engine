/*
 * Author: Team MergeConflict
 * Description: Test file for the implementation of a locked hash table
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "hash.h"
#include "queue.h"
#include "lhash.h"

typedef struct item {
    char *key;
    int value;
} item_t;

bool search(void *elementp, const void *keyp) {
    item_t *ip = elementp;
    const char *key = keyp;
    return strcmp(ip->key, key) == 0;
}

void increase(void *elementp) {
    item_t *ip = elementp;
    ip->value += 1;
}

void test_single_thread(void) {
    printf("Running single-thread locked hash tests...\n");

    /* Test 1: initialize table with two key-value pairs */
    lhashtable_t *ht = lhopen(10);
    assert(ht != NULL);

    item_t *a = malloc(sizeof(item_t));
    item_t *b = malloc(sizeof(item_t));
    assert(a != NULL && b != NULL);

    a->key = "Gent";
    a->value = 7;
    b->key = "PapaYaw";
    b->value = 12;

    assert(lhput(ht, a, a->key, (int)strlen(a->key)) == 0);
    assert(lhput(ht, b, b->key, (int)strlen(b->key)) == 0);

    /* Test 2: search for existing and missing keys */
    item_t *found = lhsearch(ht, search, "Gent", (int)strlen("Gent"));
    assert(found != NULL);
    assert(found->value == 7);

    found = lhsearch(ht, search, "PapaYaw", (int)strlen("PapaYaw"));
    assert(found != NULL);
    assert(found->value == 12);

    found = lhsearch(ht, search, "Benjamin", (int)strlen("Benjamin"));
    assert(found == NULL);

    /* Test 3: remove a key and ensure it is gone */
    item_t *removed = lhremove(ht, search, "Gent", (int)strlen("Gent"));
    assert(removed == a);

    found = lhsearch(ht, search, "Gent", (int)strlen("Gent"));
    assert(found == NULL);

    /* Test 4: apply increase to remaining elements */
    lhapply(ht, increase);

    found = lhsearch(ht, search, "PapaYaw", (int)strlen("PapaYaw"));
    assert(found != NULL);
    assert(found->value == 13);  // was 12, increased by 1

    /* Test 5: cleanup remaining elements and close table */
    removed = lhremove(ht, search, "PapaYaw", (int)strlen("PapaYaw"));
    assert(removed == b);

    free(a);
    free(b);

    lhclose(ht);

    printf("Single-thread locked hash tests passed.\n");
}

int main(void) {
    test_single_thread();

    puts("All locked hash table tests passed.\n");
    return 0;
}
