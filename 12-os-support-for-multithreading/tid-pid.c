#define _GNU_SOURCE  // Required for gettid() on Linux

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *threadFunction(void *arg) {
    printf("child thread, pid: %d, tid: %d\n", getpid(), gettid());
    pthread_exit(NULL);
}

int main() {
    pthread_t thread1, thread2;

    printf("parent thread, pid: %d, tid: %d\n", getpid(), gettid());

    if (pthread_create(&thread1, NULL, threadFunction, NULL) != 0) {
        perror("Error creating thread 1");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread2, NULL, threadFunction, NULL) != 0) {
        perror("Error creating thread 2");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(thread1, NULL) != 0) {
        perror("Error joining thread 1");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(thread2, NULL) != 0) {
        perror("Error joining thread 2");
        exit(EXIT_FAILURE);
    }

    return 0;
}