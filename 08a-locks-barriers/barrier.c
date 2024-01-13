#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define ITERATIONS      10
#define T1_SPIN_AMOUNT  200000000
#define T2_SPIN_AMOUNT  (10 * T1_SPIN_AMOUNT)

typedef struct {
    int id;
    int spin_amount;
    pthread_barrier_t *barrier;
} worker;

void *thread_fn(void *data) {
    worker *arg = (worker *)data;
    int id = arg->id;
    int iteration = 0;

    while(iteration != ITERATIONS) {

        /* busy loop to simulate activity */
        for(int i=0; i<arg->spin_amount; i++);

        printf("Thread %d done spinning, reached barrier\n", id);

        int ret = pthread_barrier_wait(arg->barrier);

        if(ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0) {
            perror("pthread_barrier_wait");
            exit(-1);
        }

        printf("Thread %d passed barrier\n", id);
        iteration++;
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    pthread_t t1, t2;
    pthread_barrier_t barrier;

    worker w1 = {1, T1_SPIN_AMOUNT, &barrier};
    worker w2 = {2, T2_SPIN_AMOUNT, &barrier};

    if(pthread_barrier_init(&barrier, NULL, 2)) {
        perror("pthread_barrier_init");
        return -1;
    }

    if(pthread_create(&t1, NULL, thread_fn, (void *)&w1) ||
        pthread_create(&t2, NULL, thread_fn, (void *)&w2)) {
        perror("pthread_create");
        return -1;
    }

    if(pthread_join(t1, NULL) ||
        pthread_join(t2, NULL))
        perror("phread_join");

    return 0;
}
