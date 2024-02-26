#define _GNU_SOURCE

#ifdef __USE_LOCKS__
#include "queue-locks.h"
#else
#include "queue-lockfree.h"
#endif /* __USE_LOCKS__ */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>

#include "params.h"

#if defined(__V3__) || defined(__V4__)
pthread_barrier_t cleanup_barrier;
extern __thread _Atomic(node *) free_pool_head; 
#endif /* defined(__V3__) || defined(__V4__) */

typedef struct {
    ub_queue *queue;
    int elem_num;
} worker;

void *thread_fn(void *data) {
    worker *w = (worker *)data;

    for(int i=0; i<w->elem_num; i++) {
        //printf("[%d] try to enqueue %d\n", gettid(), i);
        if(enqueue(w->queue, ITEM)) {
            fprintf(stderr, "cannot enqueue\n");
            exit(-1);
        }
    }

    for(int i=0; i<w->elem_num; i++) {
        int item;
        if(dequeue(w->queue, &item))
            i--;
    }

#if defined(__V3__) || defined(__V4__)
    // Empty the free pool. We need a barrier here to avoid uses after free
    pthread_barrier_wait(&cleanup_barrier);
    cleanup_free_pool();
#endif /* defined(__V3__) || defined(__V4__) */

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    ub_queue q;
    pthread_t threads[THREAD_NUM];
    worker workers[THREAD_NUM];
    struct timeval start, stop, res;

    // We want a random number of element enqueued then deqeueud in each thread,
    // but it needs to be consistent across execution so that performance
    // numbers are comparable
    srand(0);

    if(init_queue(&q)) {
        fprintf(stderr, "cannot init queue\n");
        return -1;
    }

#if defined(__V3__) || defined(__V4__)
    if(pthread_barrier_init(&cleanup_barrier, NULL, THREAD_NUM)) {
        fprintf(stderr, "cannot initialize cleanup barrier\n");
        return -1;
    }
#endif /* defined(__V3__) || defined(__V4__) */

    for(int i=0; i<THREAD_NUM; i++) {
        workers[i].queue = &q;
        workers[i].elem_num = rand()%ELEM_NUM;
    }

    gettimeofday(&start, NULL);

    for(int i=0; i<THREAD_NUM; i++)
        if(pthread_create(&threads[i], NULL, thread_fn, (void *)&workers[i]))
            errx(-1, "pthread_create");

    for(int i=0; i<THREAD_NUM; i++)
        if(pthread_join(threads[i], NULL))
            errx(-1, "pthread_join");

    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &res);

    printf("%d threads enqueue/dequeue max %d elements in "
            "%ld.%06ld s\n", THREAD_NUM, ELEM_NUM, res.tv_sec, res.tv_usec);

    destroy_queue(&q);
    return 0;
}
