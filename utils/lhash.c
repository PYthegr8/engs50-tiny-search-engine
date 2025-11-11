/*
 * lhash.c -- implements a locked hash table as an indexed set of locked queues.
 * updated by Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"
#include "hash.h"
#include "lhash.h"



struct lhashtable {
    struct hashtable *ht;
    pthread_mutex_t m;
};

lhashtable_t *lhopen(uint32_t hsize) {
    struct lhashtable *lht = malloc(sizeof(struct lhashtable));
    if (!lht) return NULL;
    lht->ht = hopen(hsize);

    if (!lht->ht) {
            free(lht);
            return NULL;
    }

    if (pthread_mutex_init(&lht->m, NULL) != 0) {
            hclose(lht->ht);
            free(lht);
            return NULL;
    }

    return (lhashtable_t *) lht;
}

void lhclose(lhashtable_t *htp) {
    struct lhashtable *lht = (struct lhashtable *)htp;
    if (!lht) return;
    pthread_mutex_destroy(&lht->m);
    hclose(lht->ht);
    free(lht);
}

int32_t lhput(lhashtable_t *htp, void *ep, const char *key, int keylen) {
    struct lhashtable *lht = (struct lhashtable *)htp;
    pthread_mutex_lock(&lht->m);
    int32_t res = hput(lht->ht, ep, key,keylen);
    pthread_mutex_unlock(&lht->m);
    return res;
}

void *lhremove(lhashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
    struct lhashtable *lht = (struct lhashtable *)htp;
    pthread_mutex_lock(&lht->m);
    void *res = hremove(lht->ht, searchfn , key, keylen);
    pthread_mutex_unlock(&lht->m);
    return res;
}

void *lhsearch(lhashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
    struct lhashtable *lht = (struct lhashtable *)htp;
    pthread_mutex_lock(&lht->m);
    void *res = hsearch(lht->ht, searchfn , key, keylen);
    pthread_mutex_unlock(&lht->m);
    return res;
}

void lhapply(lhashtable_t *htp, void (*fn)(void*)) {
    struct lhashtable *lht = (struct lhashtable *)htp;
    pthread_mutex_lock(&lht->m);
    happly(lht->ht, fn);
    pthread_mutex_unlock(&lht->m);
}

