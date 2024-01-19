/* Note that this implementation is simplified for the sake of the example and
 * not 100% correct, see here: https://www.akkadia.org/drepper/futex.pdf */

#define _GNU_SOURCE

#include "lock-bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <err.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <linux/futex.h>

atomic_int my_mutex = ATOMIC_VAR_INIT(0);

int my_mutex_lock() {
    int is_free = 0;
    int taken = 1;

    // cas(value_to_test, expected_value, new_value_to_set)
    while(!atomic_compare_exchange_strong(&my_mutex, &is_free, taken)) {
        // put the thread to sleep waiting for FUTEX_WAKE if my_mutex is still
        // equals to 1
        syscall(SYS_futex, &my_mutex, FUTEX_WAIT, 1, NULL, NULL, 0);
    }

    return 0;
}

int my_mutex_unlock() {
    atomic_store(&my_mutex, 0);
    syscall(SYS_futex, &my_mutex, FUTEX_WAKE, 1, NULL, NULL, 0);

    return 0;
}

void *thread_function(void *arg) {

    for(int i=0; i < CS_NUM; i++) {

        if (my_mutex_lock())
            err(-1, "[%d] Error locking mutex", gettid());

        if (my_mutex_unlock())
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
