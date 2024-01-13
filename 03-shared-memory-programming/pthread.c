#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NOWORKERS 5

void *thread_fn(void *arg) {
    int id = (int)(long)arg;
    printf("Thread %d running\n", id);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t workers[NOWORKERS];

    for(int i=0; i<NOWORKERS; i++)
        if(pthread_create(&workers[i], NULL, thread_fn, (void *)(long)i)) {
            perror("pthread_create");
            return -1;
        }

    for (int i = 0; i < NOWORKERS; i++)
      if(pthread_join(workers[i], NULL)) {
          perror("pthread_join");
          return -1;
      }

    printf("All done\n");
}

