// test_queue.c — minimal end-to-end tests for queue.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

static void *newi(int x){ int *p = malloc(sizeof *p); *p = x; return p; }
static bool ieq(void *e, const void *k){ return *(int*)e == *(const int*)k; }
static void times2(void *e){ *(int*)e *= 2; }


int main(void){
    // qopen / qget on empty
    queue_t *q = qopen(); assert(q);
    assert(qget(q) == NULL);

    // qput / qget FIFO
    assert(qput(q, newi(1)) == 0);
    assert(qput(q, newi(2)) == 0);
    assert(qput(q, newi(3)) == 0);
    int *a = qget(q); assert(a && *a == 1); free(a);
    int *b = qget(q); assert(b && *b == 2); free(b);
    int *c = qget(q); assert(c && *c == 3); free(c);
    assert(qget(q) == NULL);

    // qapply (modify in place), then read back
    qput(q, newi(5)); qput(q, newi(7)); qput(q, newi(9));
    qapply(q, times2);
    a = qget(q); b = qget(q); c = qget(q);
    assert(*a == 10 && *b == 14 && *c == 18);
    free(a); free(b); free(c);

    // qsearch present/absent
    qput(q, newi(10)); qput(q, newi(20)); qput(q, newi(30));
    int key20 = 20, key99 = 99;
    int *hit = qsearch(q, ieq, &key20); assert(hit && *hit == 20);
    assert(qsearch(q, ieq, &key99) == NULL);

    // qremove: middle, head, tail
    int *rm = qremove(q, ieq, &key20); assert(rm && *rm == 20); free(rm);
    int key10 = 10, key30 = 30;
    rm = qremove(q, ieq, &key10); assert(rm && *rm == 10); free(rm);
    rm = qremove(q, ieq, &key30); assert(rm && *rm == 30); free(rm);
    assert(qget(q) == NULL);

    // qconcat: q1 += q2, q2 is closed
    queue_t *q1 = qopen(); queue_t *q2 = qopen();
    qput(q1, newi(1)); qput(q1, newi(2));
    qput(q2, newi(3)); qput(q2, newi(4));
    qconcat(q1, q2);               // q2 is consumed/closed inside
    a = qget(q1); b = qget(q1); c = qget(q1); int *d = qget(q1);
    assert(*a == 1 && *b == 2 && *c == 3 && *d == 4);
    free(a); free(b); free(c); free(d);
    assert(qget(q1) == NULL);

    // cleanup remaining queue (ensure no leaks from this point)
    qclose(q1);
    qclose(q);

    puts("All queue tests passed.");
    return 0;
}
