#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define BOUNCE_LIMIT    20

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int my_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        fprintf(stderr, "World need to be >= 2 processes\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* The first process is in charge of sending the first message */
    if(my_rank == 0) {
        int payload = 0;
        int target_rank = 0;

        /* We don't want the process to send a message to itself */
        while(target_rank == my_rank)
            target_rank = rand()%world_size;

        printf("[%d] sent int payload %d to %d\n", my_rank, payload,
                target_rank);

        /* Send the message */
        MPI_Send(&payload, 1, MPI_INT, target_rank, 0, MPI_COMM_WORLD);
    }

    while(1) {
        int payload;

        /* Wait for reception of a message */
        MPI_Recv(&payload, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);

        /* Receiving -1 means we need to exit */
        if(payload == -1) {
            printf("[%d] received stop signal, exiting\n", my_rank);
            break;
        }

        if(payload == BOUNCE_LIMIT) {
            /* We have bounced enough times, send the stop signal, i.e. a
             * message with -1 as payload, to all other processes */
            int stop_payload = -1;
            printf("[%d] broadcasting stop signal\n", my_rank, payload);

            for(int i=0; i<world_size; i++)
                if(i != my_rank)
                    MPI_Send(&stop_payload, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            break;
        }

        /* increment payload */
        payload++;
        int target_rank = my_rank;

        /* Choose the next process to send a message to */
        while(target_rank == my_rank)
            target_rank = rand()%world_size;

        printf("[%d] received payload %d, sending %d to %d\n", my_rank,
                payload-1, payload, target_rank);

        MPI_Send(&payload, 1, MPI_INT, target_rank, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
