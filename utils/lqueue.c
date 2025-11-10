/*

Implementation of the lqueue module
Author: Team MergeConflict

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"
#include "lqueue.h"

typedef struct lqueue {
    queue_t *q;
    pthread_mutex_t m;
} lqueue_t;


lqueue_t *lqopen(void);
void lqclose(lqueue_t *lqp);
int32_t lqput(lqueue_t *lqp, void *elementp);
void *lqget(lqueue_t *lqp);
void lqapply(lqueue_t *lqp, void (*fn)(void* elementp));
void *lqsearch(lqueue_t *lqp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp);
void *lqremove(lqueue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp);
void lqconcat(lqueue_t *lq1p, lqueue_t *lq2p);

int main() {
    return 0;
}


lqueue_t *lqopen(void) {
    lqueue_t *lq = (lqueue_t *)malloc(sizeof(lqueue_t)); 
    if (!lq) {
        return NULL;
    }
    pthread_mutex_init(&lq->m, NULL);
    pthread_mutex_lock(&lq->m);
    lq->q = qopen();
    pthread_mutex_unlock(&lq->m);
    if (!lq->q || !lq->m){
        return NULL;
    }
    return lq;
}

void lqclose(lqueue_t *lqp) {
    pthread_mutex_lock(&lq->m);
    qclose(lqp->q);
    pthread_mutex_unlock(&lq->m);
}

int32_t lqput(lqueue_t *lqp, void *elementp) {
    pthread_mutex_lock(&lq->m);
    qput(lqp->q, elementp);
    pthread_mutex_unlock(&lq->m);
}

void *lqget(lqueue_t *lqp){
    pthread_mutex_lock(&lq->m);
    void *ep = qget(lqp->q);
    pthread_mutex_unlock(&lq->m);
    return ep;
}

void lqapply(lqueue_t *lqp, void (*fn)(void* elementp)) {
    pthread_mutex_lock(&lq->m);
    qapply(lqp->q, fn);
    pthread_mutex_unlock(&lq->m);
}

void *lqsearch(lqueue_t *lqp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp) {
    pthread_mutex_lock(&lq->m);
    void *ep = qsearch(lqp->q, searchfn, skeyp);
    pthread_mutex_unlock(&lq->m);
    return ep;
}

void *lqremove(lqueue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp) {
    pthread_mutex_lock(&lq->m);
    void *ep = qremove(lqp->q, searchfn, skeyp);
    pthread_mutex_unlock(&lq->m);
    return ep;
}

void lqconcat(lqueue_t *lq1p, lqueue_t *lq2p) {
    pthread_mutex_lock(&lq->m);
    qconcat(lq1p->q, lq2p->q);
    pthread_mutex_unlock(&lq->m);
}
