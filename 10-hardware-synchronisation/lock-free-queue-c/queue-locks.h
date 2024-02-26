#ifndef QUEUE_LOCKS_H
#define QUEUE_LOCKS_H

#include <pthread.h>

typedef struct s_node {
    int value;
    struct s_node *next;
} node;

typedef struct {
    node *head;
    node *tail;
    pthread_mutex_t enq_lock;
    pthread_mutex_t deq_lock;
} ub_queue;

node *create_node(int value);
int init_queue(ub_queue *q);
void destroy_queue(ub_queue *q);
int enqueue(ub_queue *q, int item);
int dequeue(ub_queue *q, int *res);

#endif /* QUEUE_LOCKS_H */