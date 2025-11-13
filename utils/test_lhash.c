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
    const char *key;
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


typedef struct thread_args {
    lhashtable_t *ht;
    const char **keys;
    const int *values;
    int count;
} thread_args_t;

void *thread_put(void *arg) {
    thread_args_t *args = arg;

    for (int i = 0; i < args->count; i++) {
        item_t *it = malloc(sizeof(item_t));
        assert(it != NULL);

        it->key = args->keys[i];
        it->value = args->values[i];

        int rc = lhput(args->ht, (void *)it, it->key, (int)strlen(it->key));
        assert(rc == 0);
    }

    return NULL;
}
void test_dual_thread(void) {
    printf("Running dual-thread locked hash tests...\n");

    /* Test 1: initialize shared table */
    lhashtable_t *ht = lhopen(20);
    assert(ht != NULL);

    const char *keys1[] = {"Gent", "PY", "Benjamin"};
    const int   vals1[] = {10, 20, 30};

    const char *keys2[] = {"Merge", "Conflict", "Crew"};
    const int   vals2[] = {100, 200, 300};

    thread_args_t args1 = {ht, keys1, vals1, 3};
    thread_args_t args2 = {ht, keys2, vals2, 3};

    pthread_t t1, t2;

    /* Test 2: start two threads inserting into the same table */
    int rc1 = pthread_create(&t1, NULL, thread_put, &args1);
    int rc2 = pthread_create(&t2, NULL, thread_put, &args2);
    assert(rc1 == 0 && rc2 == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    /* Test 3: verify all keys from both threads are present with correct values */

    for (int i = 0; i < 3; i++) {
        item_t *found = lhsearch(ht, search, keys1[i], (int)strlen(keys1[i]));
        assert(found != NULL);
        assert(found->value == vals1[i]);
    }

    for (int i = 0; i < 3; i++) {
        item_t *found = lhsearch(ht, search, keys2[i], (int)strlen(keys2[i]));
        assert(found != NULL);
        assert(found->value == vals2[i]);
    }

    /* Test 4: cleanup all elements and close table */
    for (int i = 0; i < 3; i++) {
        item_t *removed = lhremove(ht, search, keys1[i], (int)strlen(keys1[i]));
        assert(removed != NULL);
        free(removed);
    }

    for (int i = 0; i < 3; i++) {
        item_t *removed = lhremove(ht, search, keys2[i], (int)strlen(keys2[i]));
        assert(removed != NULL);
        free(removed);
    }

    lhclose(ht);

    printf("Dual-thread locked hash tests passed.\n");
}

int main(void) {
    test_single_thread();
    test_dual_thread();

    puts("All locked hash table tests passed.\n");
    return 0;
}
