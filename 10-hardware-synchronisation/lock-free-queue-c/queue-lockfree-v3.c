// 3rd version of the lock-free queue: we fix the use-after frees with a somehow
// hacky solution: dequeued nodes are placed in a pool to be freed later when
// the client code determines it is safe to do so (e.g. when the client code has
// some form of guarantee that there is no dequeue operation ongoing). This is
// not ideal because if such situation does not happen often, the memory
// consumption of the queue will grow. We try to alleviate that problem by
// reusing nodes from the free pools during enqueue operations, however it leads
// to a peculiar issue.
//
// This implementation is indeed still not valid: it suffers from the well-knwon
// ABA problem: https://en.wikipedia.org/wiki/ABA_problem. Imagine a thread T1
// interrupted during dequeue after it grabbed local_head (say it points to
// 0x40) and local_head_next (say it points to 0x50). Other threads can dequeue
// the node at 0x40 and it will be placed in the free list. Assume the node at
// 0x40 is reused by another enqueue operation, and following some dequeue
// operations it ends up being the head node. Assume at that point T1 finally
// resumes: the head is still pointing to 0x40 so the dequeue's CAS will
// succeed, although the value of the new head to set may not be the old
// local_head_next (0x50), which may have e.g. be placed on the free list during
// T1's interruption.

#include "queue-lockfree.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdatomic.h>

// per-thread list of freed nodes: with the __thread keyword, each thread will
// have its own copy of these 2 global variables
__thread node *free_pool_head = NULL; 
__thread node *free_pool_tail = NULL;

node *create_node(int value) {
    node *n;

    // Try to reuse a node from the free pool
    if(!free_pool_head)
        n = malloc(sizeof(node));
    else {
        n = free_pool_head;
        free_pool_head = n->next;
        if(!free_pool_head)
            free_pool_tail = NULL;
    }

    if(!n)
        return NULL;

    n->next = NULL;
    n->value = value;
    return n;
}

int init_queue(ub_queue *q) {
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

    while(1) {
        node *local_tail = q->tail;
        node *null_node = NULL;

        if(atomic_compare_exchange_weak(&local_tail->next, &null_node, n)) {
            atomic_compare_exchange_weak(&q->tail, &local_tail, n);
            return 0;
        } else {
            atomic_compare_exchange_weak(&q->tail, &local_tail, q->tail->next);
        }
    }
}

/* returns 0 on success, -1 if empty */
int dequeue(ub_queue *q, int *res) {

    while(1) {
        node *local_head = q->head;
        node *local_next_head = local_head->next;
        node *local_tail = q->tail;

        if(local_next_head == NULL)
            return -1;

        if(local_head == local_tail) {
            atomic_compare_exchange_weak(&q->tail, &local_tail, local_next_head);
            continue;
        }

        int result = local_next_head->value;

        if(atomic_compare_exchange_weak(&q->head, &local_head, local_next_head)) {

            *res = result;

            // place dequeued node in the free pool
            local_head->next = NULL;
            if(!free_pool_tail)
                free_pool_tail = free_pool_head = local_head;
            else {
                free_pool_tail->next = local_head;
                free_pool_tail = local_head;
            }

            return 0;
        }
    }
}

// Actually free the memory in the free pool. Needs to be called by each thread,
// and shouldn't be called concurrently with dequeue's operations
void cleanup_free_pool() {

    while(free_pool_head) {
        node *next = free_pool_head->next;
        free(free_pool_head);
        free_pool_head = next;
    }
    
    free_pool_tail = NULL;
}
