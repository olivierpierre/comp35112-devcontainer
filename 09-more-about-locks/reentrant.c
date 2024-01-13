#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <err.h>

#define INIT_MONEY  100000
#define ITERATIONS  100

typedef struct {
    int id;
    double balance;
    pthread_mutex_t lock;
} account;

typedef struct {
    account *a1;
    account *a2;
    int iterations;
} worker;

void initialize_account(account *a, int id, double balance) {
    a->id = id;
    a->balance = balance;

    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr))
        errx(-1, "pthread_mutexattr_init");
    if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
        errx(-1, "pthread_mutexattr_settype");
    if(pthread_mutex_init(&a->lock, &attr))
        errx(-1, "pthread_mutex_init");
}

void transfer(account *from, account *to, double amount) {

    pthread_mutex_t *lock1 = &from->lock;
    pthread_mutex_t *lock2 = &to->lock;

    if(from->id < to->id) {
        lock1 = &to->lock;
        lock2 = &from->lock;
    }

    /* Ok to take twice the same lock, it's reentrant */
    pthread_mutex_lock(lock1);
    pthread_mutex_lock(lock2);

    if(from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
    }

    pthread_mutex_unlock(lock2);
    pthread_mutex_unlock(lock1);
}

void *thread_fn(void *data) {
    worker *w = (worker *)data;

    for(int i=0; i<w->iterations; i++) {
        transfer(w->a1, w->a2, 10.0);
        transfer(w->a2, w->a1, 10.0);
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    account account1;
    pthread_t t1;

    initialize_account(&account1, 1, INIT_MONEY);
    worker w1 = {&account1, &account1, ITERATIONS};

    if(pthread_create(&t1, NULL, thread_fn, (void *)&w1))
        errx(-1, "pthread_create");

    if(pthread_join(t1, NULL))
        errx(-1, "pthread_join");

    return 0;
}
