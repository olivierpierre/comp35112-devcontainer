#ifndef QUEUE_LOCKFREE_H
#define QUEUE_LOCKFREE_H

#include <stdatomic.h>

typedef struct s_node {
    int value;
    _Atomic(struct s_node *) next;
} node;

#ifdef __V4__

#include <stdint.h>

typedef struct s_ub_queue_head {
    uintptr_t aba;
    node *node;
} ub_queue_head;

typedef struct {
    _Atomic(ub_queue_head) head;
    _Atomic(node *) tail;
} ub_queue;

#else

typedef struct {
    _Atomic(node *) head;
    _Atomic(node *) tail;
} ub_queue;

#endif /* __V4__ */

node *create_node(int value);
int init_queue(ub_queue *q);
void destroy_queue(ub_queue *q);
int enqueue(ub_queue *q, int item);
int dequeue(ub_queue *q, int *res);

#if defined(__V3__) || defined(__V4__)
void cleanup_free_pool();
#endif /* defined(__V3__) || defined(__V4__) */

#endif /* QUEUE_LOCKFREE_H */