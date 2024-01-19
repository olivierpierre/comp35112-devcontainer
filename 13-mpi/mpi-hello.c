/* On Ubuntu/Debian, to install the MPICH implementation of MPI:
 * sudo apt install mpich
 * To compile this file:
 * mpicc listing1.c -o listing1
 */

#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    /* Init. MPI */
    MPI_Init(NULL, NULL);

    /* Retrieve the world size i.e. the total number of processes */
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /* Retrieve the curent process rank (id) */
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Retreive the machine name */
    char machine_name[MPI_MAX_PROCESSOR_NAME];
    int nlen;
    MPI_Get_processor_name(machine_name, &nlen);

    printf("Hello world from processor %s, rank %d out of %d processors\n",
           machine_name, my_rank, world_size);

    /* Exit */
    MPI_Finalize();
}
