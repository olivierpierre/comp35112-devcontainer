#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000

int A[N][N];
int B[N][N];
int C[N][N];

int main(int argc, char **argv) {

    for(int i=0; i<N; i++)
        for(int j=0; j<N; j++) {
            A[i][j] = rand()%100;
            B[i][j] = rand()%100;
        }

#pragma omp parallel
    {
        for(int i=0; i<N; i++)  // this loop is parallelized automatically
            for(int j=0; j<N; j++) {
                C[i][j] = 0;
                for(int k=0; k<N; k++)
                    C[i][j] = C[i][j] + A[i][k] * B[k][j];
            }
    }

    return 0;
}
