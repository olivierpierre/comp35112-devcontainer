// 2nd version of the lockfree queue: this time we tackle the problem of the
// double memory write accesses needed by the enqueue operation. The code is
// still not valid yet: due to manual memory management in languages such as C
// (malloc/free), uses-after-free can happen in dequeue() if the operation is
// interrupted by other dequeuing threads, see dequeue()'s code for more details

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

    while(1) {
        node *local_tail = q->tail;
        node *null_node = NULL;

        if(atomic_compare_exchange_weak(&local_tail->next, &null_node, n)) {
            atomic_compare_exchange_weak(&q->tail, &local_tail, n);
            // here we use a second CAS and return even if it fails: we leave
            // the queue in the inconsistent state with q->tail pointing to the
            // old tail node and q->tail->next being non null. We'll rely on the
            // next enqueue/dequeue operation to fix the queue's state.
            return 0;
        } else {
            // the first CAS failing can mean two things: A) either our local
            // tail is invalid because another thread successfully enqueued a
            // node and in that case we just need to try again or B) the queue
            // was left in the inconsistent state by another enqueuing thread T
            // and in that case we need to fix the queue's state i.e. finishing
            // T's enqueuing operation before trying again our own enqueue op.
            // We can check if we are in A or B by checking if our local tail
            // is still valid and do the fixup if needed with a CAS
            atomic_compare_exchange_weak(&q->tail, &local_tail, q->tail->next);
        }
    }
}

/* returns 0 on success, -1 if empty */
int dequeue(ub_queue *q, int *res) {

    while(1) {
        node *local_head = q->head;
        node *local_next_head = local_head->next;
        node *local_tail = q->tail; // need the tail in case we need to fix up

        if(local_next_head == NULL)
            return -1;

        // if an enqueuing thread T1 gets interrupted by a dequeing one T2 in 
        // the middle of its 2 CAS, T2 needs first to fix up the queue's state
        // before dequeuing, otherwise on the special case of an empty queue 
        // (before T1's enqueue) T2 will dequeue the node in the process of
        // being enqueued before it is fixed up, leading to madness
        if(local_head == local_tail) {
            // if h == t and h->next is non-null we are in that special case
            atomic_compare_exchange_weak(&q->tail, &local_tail, local_next_head);
            continue;
        }

        int result = local_next_head->value;

        // This is incorrect: if a thread T1 is interrupted in dequeue after
        // grabbing local_head by another thread T2 that successully dequeues,
        // T2 will free the old head which will trigger a use-after-free when T1
        // resumes and accesses local_next_head e.g. to check if the queue is
        // empty. We can also have T1 interrupted before setting the value of
        // result by a series of 2+ successful dequeues, and then setting result
        // will lead to a use-after-free once again
        if(atomic_compare_exchange_weak(&q->head, &local_head, local_next_head)) {
            free(local_head);
            *res = result;
            return 0;
        }
    }
}
