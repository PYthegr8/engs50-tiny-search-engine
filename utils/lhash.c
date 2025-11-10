/*
 * hash.c -- implements a locked hash table as an indexed set of locked queues.
 * updated by Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"
#include "hash.h"
#include "lhash.h"


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



lhashtable_t *lhopen(uint32_t hsize) {
}


lhashtable_t *lhopen(uint32_t hsize) {
}

void lhclose(lhashtable_t *htp) {
}

int32_t lhput(lhashtable_t *htp, void *ep, const char *key, int keylen) {
}

void *lhremove(lhashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
}

void *lhsearch(lhashtable_t *htp, bool (*searchfn)(void*, const void*), const char *key, int32_t keylen) {
}

void lhapply(lhashtable_t *htp, void (*fn)(void*)) {
}

