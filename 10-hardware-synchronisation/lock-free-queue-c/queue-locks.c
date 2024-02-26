// Lock-based implementation of the queue

#include "queue-locks.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

node *create_node(int value) {
    node *n = malloc(sizeof(node));

    if(!n)
        return NULL;

    n->next = NULL;
    n->value = value;
    return n;
}

int init_queue(ub_queue *q) {
    if(pthread_mutex_init(&q->enq_lock, NULL) ||
            pthread_mutex_init(&q->deq_lock, NULL))
        return -1;

    node *n = create_node(0);

    if(!n)
        return -ENOMEM;

    q->head = n;
    q->tail = n;

    return 0;
}

void destroy_queue(ub_queue *q) {
    node *n = q->head;

    do {
        node *to_free = n;
        n = n->next;
        free(to_free);
    } while(n);
}

int enqueue(ub_queue *q, int item) {
    node *n = create_node(item);
    if(!n)
        return -ENOMEM;

    pthread_mutex_lock(&q->enq_lock);
    q->tail->next = n;
    q->tail = n;
    pthread_mutex_unlock(&q->enq_lock);

    return 0;
}

/* returns 0 on success, -1 if empty */
int dequeue(ub_queue *q, int *res) {
    int ret = -1;

    pthread_mutex_lock(&q->deq_lock);
    if(q->head->next) {
        node *old = q->head;
        q->head = q->head->next;
        free(old);
        *res = q->head->value;
        ret = 0;
    }
    pthread_mutex_unlock(&q->deq_lock);

    return ret;
}
