#define _GNU_SOURCE

#include "lock-bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <err.h>
#include <sys/time.h>

#define KEY         1234    // key for the system V semaphore

// Semaphore union for semctl
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int sem_id;

void create_semaphore() {
    // Create a semaphore and initialize it with value 1
    sem_id = semget(KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1)
        err(-1, "Error creating semaphore");

    union semun arg;
    arg.val = 1;

    if (semctl(sem_id, 0, SETVAL, arg) == -1)
        err(-1, "Error initializing semaphore");
}

void *thread_function(void *arg) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_flg = 0;

    for(int i=0; i < CS_NUM; i++) {

        sem_op.sem_op = -1; // Lock
        if (semop(sem_id, &sem_op, 1) == -1)
            err(-1, "[%d] Error locking semaphore", gettid());

        // Release the semaphore
        sem_op.sem_op = 1; // Unlock

        if (semop(sem_id, &sem_op, 1) == -1)
            err(-1, "[%d] Error unlocking semaphore", gettid());
    }

    return NULL;
}

int main() {
    struct timeval start, stop, total;

    create_semaphore();

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

    double throughput_us = (double)CS_NUM*THREAD_NUM / (double)(total.tv_usec + total.tv_sec*1000000);
    printf("%d threads ran a total of %d crit. sections in %ld.%06ld seconds, "
    "throughput: %lf cs/usec\n",
    THREAD_NUM, CS_NUM*THREAD_NUM, total.tv_sec, total.tv_usec, throughput_us);

    // Remove the semaphore
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("Error removing semaphore");
        exit(EXIT_FAILURE);
    }

    return 0;
}
