#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <err.h>

#define INIT_MONEY  100000
#define ITERATIONS  100

typedef struct {
    double balance;
    pthread_mutex_t lock;
} account;

typedef struct {
    account *a1;
    account *a2;
    int iterations;
} worker;

void initialize_account(account *a, double balance) {
    a->balance = balance;
    if(pthread_mutex_init(&a->lock, NULL))
        errx(-1, "pthread_mutex_init");    // same as perror(msg); exit(-1);
}

void transfer(account *from, account *to, double amount) {
    if(from == to)
        return;

    /* BUGGY code: risk of deadlock when a thread calls transfer(a, b) while
     * another calls transfer(b, a) */

    pthread_mutex_lock(&from->lock);
    pthread_mutex_lock(&to->lock);

    if(from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
    }

    pthread_mutex_unlock(&to->lock);
    pthread_mutex_unlock(&from->lock);
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
    account account1, account2;
    pthread_t t1, t2;

    initialize_account(&account1, INIT_MONEY);
    initialize_account(&account2, INIT_MONEY);

    worker w1 = {&account1, &account2, ITERATIONS};
    worker w2 = {&account2, &account1, ITERATIONS};

    if(pthread_create(&t1, NULL, thread_fn, (void *)&w1) ||
            pthread_create(&t2, NULL, thread_fn, (void *)&w2))
        errx(-1, "pthread_create");

    if(pthread_join(t1, NULL) || pthread_join(t2, NULL))
        errx(-1, "pthread_join");

    return 0;
}
