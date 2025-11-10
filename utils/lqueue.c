/*
 * lqueue.c -- implementation of the lock queue module
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"
#include "lqueue.h"

/*
Internal struct for our locked queue
*/
typedef struct lqueue_internal {
    struct queue *q;
    pthread_mutex_t m;
} lqueue_internal_t;

lqueue_t *lqopen(void) {
    lqueue_internal_t *lq = malloc(sizeof(lqueue_internal_t));
    if (!lq) return NULL;

    lq->q = qopen();
    if (!lq->q) {
        free(lq);
        return NULL;
    }

    if (pthread_mutex_init(&lq->m, NULL) != 0) {
        qclose(lq->q);
        free(lq);
        return NULL;
    }

    return (lqueue_t *) lq;
}

void lqclose(lqueue_t *lqp) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    if (!lq) return;

    pthread_mutex_destroy(&lq->m);
    qclose(lq->q);
    free(lq);
}

int32_t lqput(lqueue_t *lqp, void *elementp) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    pthread_mutex_lock(&lq->m);
    int32_t res = qput(lq->q, elementp);
    pthread_mutex_unlock(&lq->m);
    return res;
}

void *lqget(lqueue_t *lqp) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    pthread_mutex_lock(&lq->m);
    void *res = qget(lq->q);
    pthread_mutex_unlock(&lq->m);
    return res;
}

void lqapply(lqueue_t *lqp, void (*fn)(void *elementp)) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    pthread_mutex_lock(&lq->m);
    qapply(lq->q, fn);
    pthread_mutex_unlock(&lq->m);
}

void *lqsearch(lqueue_t *lqp,
               bool (*searchfn)(void *elementp, const void *keyp),
               const void *skeyp) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    pthread_mutex_lock(&lq->m);
    void *res = qsearch(lq->q, searchfn, skeyp);
    pthread_mutex_unlock(&lq->m);
    return res;
}

void *lqremove(lqueue_t *lqp,
               bool (*searchfn)(void *elementp, const void *keyp),
               const void *skeyp) {
    lqueue_internal_t *lq = (lqueue_internal_t *) lqp;
    pthread_mutex_lock(&lq->m);
    void *res = qremove(lq->q, searchfn, skeyp);
    pthread_mutex_unlock(&lq->m);
    return res;
}

void lqconcat(lqueue_t *lq1p, lqueue_t *lq2p) {
    lqueue_internal_t *lq1 = (lqueue_internal_t *) lq1p;
    lqueue_internal_t *lq2 = (lqueue_internal_t *) lq2p;

    if (!lq1 || !lq2) return;

    pthread_mutex_lock(&lq1->m);
    pthread_mutex_lock(&lq2->m);

    qconcat(lq1->q, lq2->q);

    pthread_mutex_unlock(&lq2->m);
    pthread_mutex_unlock(&lq1->m);

    pthread_mutex_destroy(&lq2->m);
    free(lq2);
}

