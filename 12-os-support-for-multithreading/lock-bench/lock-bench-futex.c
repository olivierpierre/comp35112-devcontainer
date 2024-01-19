// Compile with gcc lock-bench.c -o lock-bench -lpthread
#define _GNU_SOURCE

#include "lock-bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <err.h>
#include <sys/time.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_function(void *arg) {

    for(int i=0; i < CS_NUM; i++) {

        if (pthread_mutex_lock(&mutex))
            err(-1, "[%d] Error locking mutex", gettid());

        // Release the semaphore
        if (pthread_mutex_unlock(&mutex))
            err(-1, "[%d] Error unlocking mutex", gettid());
    }

    return NULL;
}

int main() {
    struct timeval start, stop, total;

    pthread_t threads[THREAD_NUM];

    gettimeofday(&start, NULL);

    for(int i=0; i<THREAD_NUM; i++)
        if(pthread_create(&threads[i], NULL, thread_function, NULL) != 0) {
            perror("Error creating threads");
            exit(EXIT_FAILURE);
        }

    for(int i=0; i<THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &total);

    int throughput_us = CS_NUM*THREAD_NUM / (total.tv_usec + total.tv_sec*1000000);
    printf("%d threads ran a total of %d crit. sections in %ld.%06ld seconds, "
    "throughput: %d cs/usec\n",
    THREAD_NUM, CS_NUM*THREAD_NUM, total.tv_sec, total.tv_usec, throughput_us);

    return 0;
}
