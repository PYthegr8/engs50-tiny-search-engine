/*
Implementation of queue.c module
Engs050 Module 3
Team members: Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"

typedef struct node {
    void *data;
    struct node *next;
} node_t;

struct queue {
    node_t *front;
    node_t *back;
    int size;
};

queue_t *qopen(void);
void qclose(queue_t *qp);
int32_t qput(queue_t *qp, void *elementp);
void *qget(queue_t *qp);
void qapply(queue_t *qp, void (*fn)(void* elementp));
void *qsearch(queue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp);
void *qremove(queue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp);
void qconcat(queue_t *q1p, queue_t *q2p);
void node_free(node_t *node);
node_t *node_init(void *elementp);

queue_t *qopen(void) {
    struct queue *q = malloc(sizeof(struct queue));
    if (!q) {
        fprintf(stderr, "[Error, malloc failed allocating the queue!\n]");
        return NULL;
    }
    q->front = NULL;
    q->back = NULL;
    q->size = 0;
    return (queue_t *)q;
}

void qclose(queue_t *qp) {
    struct queue *q = (struct queue *)qp;
    if (!q){
        fprintf(stderr, "Error: Invalid queue passed in\n");
        return;
    }
    node_t *curr_node = q->front;
    node_t *next_node;

    while (curr_node != NULL) {
        next_node = curr_node->next;
        node_free(curr_node);
        curr_node = next_node;
    }
    free(q);
}

int32_t qput(queue_t *qp, void *elementp) {
    struct queue *q = (struct queue *)qp;
    if (!q || !elementp){
        fprintf(stderr, "Error: Invalid input");
        return 1;
    }
    node_t *new_node = node_init(elementp);
    if (!new_node) return 1;
    if (!q->front) {
        q->front = q->back = new_node;
    }
    else {
        q->back->next = new_node;
        q->back = q->back->next;
    }
    q->size++;
    return 0;
}

void *qget(queue_t *qp){
    struct queue *q = (struct queue *)qp;
    if (!q || !q->front) {
        return NULL;
    }
    node_t *node_to_return = q->front;
    void *data = node_to_return->data;

    q->front = node_to_return->next;
    node_to_return->next = NULL;
    if (!q->front) {
        q->back = NULL;
    }

    node_free(node_to_return);
    q->size--;
    return data;
}

// Helper function to free individual nodes
void node_free(node_t *node) {
    if (node != NULL) {
        free(node);
    }
}

node_t *node_init(void *elementp){
    node_t *node = malloc(sizeof(node_t));
    if (!node) {
        fprintf(stderr, "[Error: Malloc failed allocating node!\n]");
        return NULL;
    }
    node->data = elementp;
    node->next = NULL;
    return node;
}

void qapply(queue_t *qp, void (*fn)(void* elementp)){
     struct queue *q = (struct queue *)qp;

     if (!q || !fn){
        fprintf(stderr, "Error: Invalid input\n");
        return ;
     }

     node_t *curr_node = q->front;
     node_t *next_node;

     while (curr_node != NULL) {
        next_node = curr_node->next;
        fn(curr_node->data);
        curr_node = next_node;
     }
     return;
}

void *qsearch(queue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp){
     struct queue *q = (struct queue *)qp;

     if (!q || !searchfn){
        fprintf(stderr, "Error: Invalid input\n");
        return NULL;
     }

     node_t *curr_node = q->front;
     node_t *next_node;

     while (curr_node != NULL) {
        next_node = curr_node->next;

         if (searchfn(curr_node->data, skeyp)){
            return curr_node -> data;
         }
         curr_node = next_node;
     }
     return NULL;
}

void *qremove(queue_t *qp, bool (*searchfn)(void *elementp, const void *keyp), const void *skeyp) {
    struct queue *q = (struct queue *)qp;

    if (!q || !searchfn) {
        fprintf(stderr, "Error: Invalid input\n");
        return NULL;
    }

    node_t *curr_node = q->front;
    node_t *next_node;
    node_t *trailing_node = NULL;

    while (curr_node != NULL) {
        next_node = curr_node->next;

        if (searchfn(curr_node->data, skeyp)) {
            void *data_to_return = curr_node->data;

            if (trailing_node == NULL) {
                q->front = next_node;
            } 

            else {
                trailing_node->next = next_node;
            }

            if (next_node == NULL) {
                q->back = trailing_node;
            }

            node_free(curr_node);
            if (q->size > 0) q->size--;
            return data_to_return;
        }

        trailing_node = curr_node;
        curr_node = next_node;
    }

    return NULL;
}


void qconcat(queue_t *q1p, queue_t *q2p) {
    if (q1p == NULL || q2p == NULL || q1p == q2p) return;

    void *elem;
    while ((elem = qget(q2p)) != NULL) {
        (void)qput(q1p, elem);
    }
    qclose(q2p);
}


