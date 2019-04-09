#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifdef QUIET
#define printf(...)
#endif




#define MAX_RANK        128
int sbuf[2];
int rbuf[2*MAX_RANK];

#define ROOT 2


int main(int argc,char *argv[])
{
    int max_rank;
    int rank;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (max_rank > MAX_RANK) {
        printf("Too many nodes.\n");
        exit(1);
    }


    sbuf[0] = rank*10;
    sbuf[1] = max_rank-rank;
//    printf("core %d send %d and %d\n", rank, sbuf[0], sbuf[1]);
    MPI_Gather(sbuf, 2, MPI_INT, rbuf, 2, MPI_INT, ROOT, MPI_COMM_WORLD);

    if (rank==ROOT) {
        for(i=0; i<2*max_rank; i++)
            printf("%d ", rbuf[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

