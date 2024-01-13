#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

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

void deposit(bounded_buffer *b, int message) {
    pthread_mutex_lock(&b->lock);

    int full = (b->count == b->max_elements);

    while(full) {
        pthread_mutex_unlock(&b->lock);
        usleep(100);
        pthread_mutex_lock(&b->lock);
        full = (b->count == b->max_elements);
    }

    b->buffer[b->in_index] = message;
    b->in_index = (b->in_index + 1) % b->max_elements;
    b->count++;

    pthread_mutex_unlock(&b->lock);
}

int extract(bounded_buffer *b) {
    pthread_mutex_lock(&b->lock);

    int empty = !(b->count);

    while(empty) {
        pthread_mutex_unlock(&b->lock);
        usleep(100);
        pthread_mutex_lock(&b->lock);
        empty = !(b->count);
    }

    int message = b->buffer[b->out_index];
    b->out_index = (b->out_index + 1) % b->max_elements;
    b->count--;

    pthread_mutex_unlock(&b->lock);
    return message;
}

void *deposit_thread_fn(void *data) {
    worker *w = (worker *)data;

    for(int i=0; i<w->iterations; i++) {
        deposit(w->bb, i);
        printf("[deposit thread] put %d\n", i);
    }

    pthread_exit(NULL);
}

void *extract_thread_fn(void *data) {
    worker *w = (worker *)data;

    for(int i=0; i<w->iterations; i++) {
        int x = extract(w->bb);
        printf("[extract thread] got %d\n", x);
    }

    pthread_exit(NULL);
}

#define BUFFER_SIZE 100

int main(int argc, char **argv) {
    bounded_buffer b;
    pthread_t t1, t2;

    if(init_bounded_buffer(&b, BUFFER_SIZE)) {
        fprintf(stderr, "cannot initialize bounded buffer\n");
        return -1;
    }

    worker w1 = {BUFFER_SIZE*2, &b};
    worker w2 = {BUFFER_SIZE*2, &b};

    if(pthread_create(&t1, NULL, deposit_thread_fn, (void *)&w1) ||
            pthread_create(&t2, NULL, extract_thread_fn, (void *)&w2)) {
        perror("pthread_create");
        return -1;
    }

    if(pthread_join(t1, NULL) || pthread_join(t2, NULL))
        perror("pthread_join");

    destroy_bounded_buffer(&b);
    return 0;
}
