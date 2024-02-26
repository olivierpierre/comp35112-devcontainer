// Naive and incorrect attempt at implementing the lock-free queue in C, taking
// as a basis the java implementation

#include "queue-lockfree.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdatomic.h>

node *create_node(int value) {
    node *n = malloc(sizeof(node));

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

    // Naive attempt at implementing a lock-free enqueue operation: get a local
    // reference to the tail then 1) enqueue with CAS the node at the end of the
    // queue if our local reference is still valid and 2) update the queue's
    // tail with CAS if our local reference is still valid
    while(1) {
        node *local_tail = q->tail;
        node *null_node = NULL;

        if(atomic_compare_exchange_weak(&q->tail->next, &null_node, n)) {
            // This does not work in C! Assume we are interrupted right after
            // the CAS above. Because that CAS suceeded we know nobody can
            // enqueue until we fix the tail, but other threads can still
            // dequeue. If enough dequeue operations happen, the tail will be
            // freed and the statement below will be a use after free.
            q->tail = n;
            return 0;
        }
    }
}

/* returns 0 on success, -1 if empty */
int dequeue(ub_queue *q, int *res) {

    while(1) {
        // get local copies of the head node reference, and the next node too
        // so we can check if it's NULL (i.e. queue empty) and later use it to
        // set the new head
        node *local_head = q->head;
        node *local_next_head = local_head->next;

        // queue empty
        if(local_next_head == NULL)
            return -1;

        // The value we'll return if we succeed with the dequeue operation
        int result = local_next_head->value;

        // the head may have been modified since we grabbed our local copies,
        // ensure it is not the case and update the head reference to the next
        // node if we still reference the valid head with a CAS
        if(atomic_compare_exchange_weak(&q->head, &local_head, local_next_head)) {

            // CAS succeeded, free the node data structure and return the value
            free(local_head);
            *res = result;
            return 0;
        }

        // CAS failed, we don't hold a reference to the valid head so we need
        // to start again.
    }
}