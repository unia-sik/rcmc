#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifdef QUIET
#define printf(...)
#endif



#define MAX_RANK        128
int sbuf[2*MAX_RANK];
int rbuf[2];

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

    for(i=0; i<max_rank; i++) {
        if (rank==ROOT) {
            sbuf[2*i] = 10*i;
            sbuf[2*i+1] = max_rank-i;
        }
    }
    MPI_Scatter(sbuf, 2, MPI_INT, rbuf, 2, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("rank=%d:%d/%d\n", rank, rbuf[0], rbuf[1]);

    MPI_Finalize();
    return 0;
}

