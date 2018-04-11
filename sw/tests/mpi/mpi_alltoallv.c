#include <mpi.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef QUIET
#define printf(...)
#endif

#define MAX_RANK 1024
#define MAX_BUF  (4*MAX_RANK)

int scounts[MAX_RANK];
int rcounts[MAX_RANK];
int sdisp[MAX_RANK];
int rdisp[MAX_RANK];
uint16_t sray[MAX_BUF];
uint16_t rray[MAX_BUF];


static uint64_t seed;

void my_srand(unsigned s)
{
    seed = s-1;
}

int my_rand()
{
    seed = 6364136223846793005ULL*seed + 1;
    return seed>>33;
}


int main(int argc, char *argv[])
{
    int max_rank, rank;
    int ssize, rsize, i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (max_rank > MAX_RANK) {
        if (rank==0) printf("Too many cores\n");
        MPI_Finalize();
        return 1;
    }

    // seed the random number generator with a different number on each core
    my_srand(rank+1);

    // find out how much data to send
    for (i=0; i<max_rank; i++) {
        scounts[i] = (my_rand() % 5) + 1;
    }
    printf("rank= %d scounts=", rank);
    for(i=0;i<max_rank;i++) printf("%d ", scounts[i]);

    // tell the other processors how much data is coming
    MPI_Alltoall(scounts, 1, MPI_INT, rcounts, 1, MPI_INT, MPI_COMM_WORLD);

    // calculate displacements and the size of the arrays
    sdisp[0] = 0;
    rdisp[0] = 0;
    ssize = scounts[max_rank-1];
    rsize = rcounts[max_rank-1];
    for (i=1; i<max_rank; i++){
        sdisp[i] = scounts[i-1] + sdisp[i-1];
        rdisp[i] = rcounts[i-1] + rdisp[i-1];
        ssize = ssize + scounts[i-1];
        rsize = rsize + rcounts[i-1];
    }

    if (ssize > MAX_BUF || rsize > MAX_BUF) {
        printf("Out of memory\n");
        MPI_Abort(MPI_COMM_WORLD, 2);
        return 2;
    }
    for (i=0; i<ssize; i++) sray[i]=rank;

    printf("\n");
    // send/recv different amounts of data to/from each processor
    MPI_Alltoallv(sray, scounts, sdisp, MPI_INT16_T, rray, rcounts, rdisp, 
        MPI_INT16_T, MPI_COMM_WORLD);

    printf("rank=%d array=", rank);
    for(i=0; i<rsize; i++) printf("%"PRIu16" ",rray[i]);
    printf("\n");

    return MPI_Finalize();
}
