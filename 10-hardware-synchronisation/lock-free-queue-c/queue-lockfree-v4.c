// 4th version of the lock-free queue. We use the 16 bytes long CAS provided
// by modern x86-64 CPUs and available with C11 to perform the dequeue CAS
// jointly on 1) the head address and 2) a magic value incremented each time
// the head is changed. For that we need the head data structure to be updated
// to include that magic value, which needs to be 8 bytes long, the other member
// being the actual pointer to the head node, for a total of 16 bytes.
//
// This is adapted from the lock-free stack implementation described here:
// https://nullprogram.com/blog/2014/09/02/

#include "queue-lockfree.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdatomic.h>

__thread node *free_pool_head = NULL; 
__thread node *free_pool_tail = NULL;

node *create_node(int value) {
    node *n;

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
    ub_queue_head h = {0, n}; // initialize aba at 0

    if(!n)
        return -ENOMEM;

    q->head = h;
    q->tail = n;

    return 0;
}

void destroy_queue(ub_queue *q) {
    // Here we first need to grab a local, non-atomic copy of the head data
    // structure, before being able to access the node member
    ub_queue_head local_head = q->head;
    node *n = local_head.node;

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
        // In this loop the node * pointers for local_head and local_head next
        // have been switched to the new data structure. It is *not* a pointer
        // as we want the CAS to operate the 16 bytes of the data structure,
        // and not the 8 bytes of a pointer to it
        ub_queue_head local_head = q->head;
        ub_queue_head local_next_head;
        local_next_head.aba = local_head.aba + 1;
        local_next_head.node = local_head.node->next;
        node *local_tail = q->tail;

        if(local_next_head.node == NULL)
            return -1;

        if(local_head.node == local_tail) {
            atomic_compare_exchange_weak(&q->tail, &local_tail, local_next_head.node);
            continue;
        }

        int result = local_next_head.node->value;

        // This is now a 16 bytes CAS, made on both the aba and node values of
        // the head data structure. In case of ABA, it will fail as, although
        // the node value of local_head and q->head is the same, the aba member
        // will be different because it changes each time the head is changed
        if(atomic_compare_exchange_weak(&q->head, &local_head, local_next_head)) {

            *res = result;

            node *old_head = local_head.node;
            old_head->next = NULL;
            if(!free_pool_tail)
                free_pool_tail = free_pool_head = old_head;
            else {
                free_pool_tail->next = old_head;
                free_pool_tail = old_head;
            }

            return 0;
        }
    }
}

void cleanup_free_pool() {

    while(free_pool_head) {
        node *next = free_pool_head->next;
        free(free_pool_head);
        free_pool_head = next;
    }
    
    free_pool_tail = NULL;
}
