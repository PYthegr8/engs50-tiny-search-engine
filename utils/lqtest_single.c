/*
 * lqtest_single.c -- single-threaded test for lqueue
 */
#include <stdio.h>
#include <stdlib.h>
#include "lqueue.h"

void print_elem(void *elem) {
    printf("%d ", *(int *)elem);
}

int main() {
    printf("=== Single-threaded lqueue test ===\n");

    lqueue_t *lq = lqopen();
    if (!lq) {
        fprintf(stderr, "Failed to open lqueue\n");
        return 1;
    }

    int *a = malloc(sizeof(int)); *a = 10;
    int *b = malloc(sizeof(int)); *b = 20;
    int *c = malloc(sizeof(int)); *c = 30;

    lqput(lq, a);
    lqput(lq, b);
    lqput(lq, c);

    printf("Applied print function: ");
    lqapply(lq, print_elem);
    printf("\n");

    int *first = lqget(lq);
    printf("Got first element: %d\n", *first);
    free(first);

    printf("Queue after get: ");
    lqapply(lq, print_elem);
    printf("\n");

    lqclose(lq);
    printf("Single-threaded test complete.\n");
    return 0;
}

