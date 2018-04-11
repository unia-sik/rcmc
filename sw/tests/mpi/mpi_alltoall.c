#include <mpi.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef QUIET
#define printf(...)
#endif


#define MAX_RANK        128
uint64_t scounts[MAX_RANK];
uint64_t rcounts[MAX_RANK];


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

//    delay(10000*rank);
    printf("rank=%d send ", rank);
    for(i=0; i<max_rank; i++) {
        scounts[i] = rank*1000+i;
        printf("%"PRIu64" ", scounts[i]);
    }
    printf("\n");

    MPI_Alltoall(scounts, 1, MPI_INT64_T, rcounts, 1, MPI_INT64_T, 
        MPI_COMM_WORLD);

//    delay(10000*rank);
    printf("rank=%d recv ", rank);
    for(i=0; i<max_rank; i++) {
        printf("%"PRIu64" ", rcounts[i]);
    }
    printf("\n");

//    delay(10000000*rank);
    printf("rank=%d barrier\n", rank);
    
    MPI_Barrier(MPI_COMM_WORLD);
    printf("rank=%d finished\n", rank);


    MPI_Finalize();
    exit(0);
    return 0;
}

