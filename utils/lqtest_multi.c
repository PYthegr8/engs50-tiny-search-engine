/*
 * lqtest_multi.c -- dual-threaded test for lqueue
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lqueue.h"

#define NUM_ITEMS 1000

typedef struct {
    lqueue_t *lq;
    int id;
} thread_arg_t;

void *producer(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        int *val = malloc(sizeof(int));
        *val = i;
        lqput(t->lq, val);
    }
    printf("Producer %d done.\n", t->id);
    return NULL;
}

void *consumer(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;
    int count = 0;
    while (count < NUM_ITEMS) {
        int *val = (int *)lqget(t->lq);
        if (val != NULL) {
            count++;
            free(val);
        }
    }
    printf("Consumer %d got %d items.\n", t->id, count);
    return NULL;
}

int main() {
    printf("=== Multi-threaded lqueue test ===\n");

    lqueue_t *lq = lqopen();
    if (!lq) {
        fprintf(stderr, "Failed to open lqueue\n");
        return 1;
    }

    pthread_t prod_thread, cons_thread;
    thread_arg_t arg = { .lq = lq, .id = 1 };

    pthread_create(&prod_thread, NULL, producer, &arg);
    pthread_create(&cons_thread, NULL, consumer, &arg);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    lqclose(lq);
    printf("Multi-threaded test complete.\n");
    return 0;
}

