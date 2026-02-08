#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define N 2000

int A[N][N];
int B[N][N];
int C[N][N];

int main(int argc, char **argv) {
    struct timeval start, end, res;

    for(int i=0; i<N; i++)
        for(int j=0; j<N; j++) {
            A[i][j] = rand()%100;
            B[i][j] = rand()%100;
        }

    gettimeofday(&start, NULL);

#pragma omp parallel for
    for(int i=0; i<N; i++)  // this loop is parallelized automatically
        for(int j=0; j<N; j++) {
            C[i][j] = 0;
            for(int k=0; k<N; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
    }

    gettimeofday(&end, NULL);
    timersub(&end, &start, &res);

    printf("%d x %d matrix multiplication (OMP): %ld.%06ld\n", N, N, res.tv_sec, res.tv_usec);

    return 0;
}
