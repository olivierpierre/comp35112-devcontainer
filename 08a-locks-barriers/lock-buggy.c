#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Buggy implementation of the bounded buffer, omitting locks. This program
// initialises the buffer with 5000 slots and creates two threads that insert
// 2500 element each, with each element inserted being 0, 1, 2, ..., 2499. Once
// the threads are done the main thread extracts and prints every element from
// the bounded buffer. That extract operation shows that 1) some elements are
// missing and 2) due to that the main thread blocks as it fails to extract
// the expected 5000 elements.

typedef struct {
    int *buffer;            // the buffer
    int max_elements;       // size of the buffer
    int in_index;           // index of the next free slot
    int out_index;          // index of the next message to extract
    int count;              // number of used slots
    pthread_mutex_t lock;   // lock protecting the buffer
} bounded_buffer;

typedef struct {
    int iterations;
    bounded_buffer *bb;
} worker;

int init_bounded_buffer(bounded_buffer *b, int size) {
    b->buffer = malloc(size * sizeof(int));
    if(!b->buffer)
        return -1;

    b->max_elements = size;
    b->in_index = 0;
    b->out_index = 0;
    b->count = 0;
    if(pthread_mutex_init(&b->lock, NULL))
        return -1;

    return 0;
}

void destroy_bounded_buffer(bounded_buffer *b) {
    free(b->buffer);
}

/* BUGGY version of deposit, without locks */
void deposit(bounded_buffer *b, int message) {

    while (b->count == b->max_elements);

    b->buffer[b->in_index] = message;
    b->in_index = (b->in_index + 1) % b->max_elements;
    b->count++;
}

/* BUGGY version of extract, without locks */
int extract(bounded_buffer *b) {

    while (!(b->count));

    int message = b->buffer[b->out_index];
    b->out_index = (b->out_index + 1) % b->max_elements;
    b->count--;

    return message;
}

void *thread_fn(void *data) {
    worker *w = (worker *)data;

    for(int i=0; i<w->iterations; i++)
        deposit(w->bb, i);

    pthread_exit(NULL);
}

#define BUFFER_SIZE 5000

int main(int argc, char **argv) {
    bounded_buffer b;
    pthread_t t1, t2;

    if(init_bounded_buffer(&b, BUFFER_SIZE)) {
        fprintf(stderr, "cannot initialize bounded buffer\n");
        return -1;
    }

    worker w1 = {BUFFER_SIZE/2, &b};
    worker w2 = {BUFFER_SIZE/2, &b};

    if(pthread_create(&t1, NULL, thread_fn, (void *)&w1) ||
            pthread_create(&t2, NULL, thread_fn, (void *)&w2)) {
        perror("pthread_create");
        return -1;
    }

    if(pthread_join(t1, NULL) || pthread_join(t2, NULL)) {
        perror("pthread_join");
        return -1;
    }

    for(int i=0; i<BUFFER_SIZE; i++)
        printf("%d ", extract(&b));

    destroy_bounded_buffer(&b);
    return 0;
}
