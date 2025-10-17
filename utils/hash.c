/*
 * hash.c -- implements a generic hash table as an indexed set of queues.
 * updated by Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"
#include "hash.h"

#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash(const char *data, int len, uint32_t tablesize) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL)
        return 0;

    rem = len & 3;
    len >>= 2;

    for (; len > 0; len--) {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    switch (rem) {
        case 3: hash += get16bits(data);
                hash ^= hash << 16;
                hash ^= data[sizeof(uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits(data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash % tablesize;
}

struct hashtable {
    struct queue **buckets;
    uint32_t hsize;
};

hashtable_t *hopen(uint32_t hsize) {
    struct hashtable *ht = malloc(sizeof(struct hashtable));
    if (!ht) return NULL;

    ht->buckets = (struct queue **)malloc(hsize * sizeof(struct queue *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->hsize = hsize;
    for (uint32_t i = 0; i < hsize; i++) {
        ht->buckets[i] = qopen();
    }
    return (hashtable_t *)ht;
}

void hclose(hashtable_t *htp) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (!ht) return;
    for (uint32_t i = 0; i < ht->hsize; i++) {
        if (ht->buckets[i]) qclose(ht->buckets[i]);
    }
    free(ht->buckets);
    free(ht);
}

int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (!ht || !ep || !key) return 1;
    uint32_t idx = SuperFastHash(key, keylen, ht->hsize);
    qput(ht->buckets[idx], ep);
    return 0;
}

void *hremove(hashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (!ht || !searchfn || !key) return NULL;
    uint32_t idx = SuperFastHash(key, keylen, ht->hsize);
    return qremove(ht->buckets[idx], searchfn, key);
}

void *hsearch(hashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (!ht || !searchfn || !key) return NULL;
    uint32_t idx = SuperFastHash(key, keylen, ht->hsize);
    return qsearch(ht->buckets[idx], searchfn, key);
}

void happly(hashtable_t *htp, void (*fn)(void*)) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (!ht || !fn) return;
    for (uint32_t i = 0; i < ht->hsize; i++) {
        qapply(ht->buckets[i], fn);
    }
}
