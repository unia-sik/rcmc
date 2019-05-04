#include "mpi.h"
#include <stdlib.h>
#include "debug.h"

#define MAX_RANK        128
int recv[MAX_RANK];

void report(char ch)
{
    Debug(ch);
    Debug(10);
    MPI_Finalize();
    exit(0);
}

int main(int argc,char *argv[])
{
    int max_rank;
    int rank;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (max_rank > MAX_RANK) report('M'); // too many nodes

    uint64_t data[2*max_rank];

    for (i=0; i<max_rank; i++) data[2*i] = rank;

    int sendcounts[max_rank];
    int sdispls[max_rank];
    int recvcounts[max_rank];
    int rdispls[max_rank];

    for (int i=0; i<max_rank; i++) {
        sendcounts[i] = 1;
        sdispls[i] = i * 8 * 2;
        recvcounts[i] = 1;
        rdispls[i] = i * 8 * 3;
    }

    MPI_Alltoallv(data, sendcounts, sdispls, MPI_INT64_T, recv, recvcounts, rdispls, MPI_INT64_T, MPI_COMM_WORLD);

    for (i=0; i<max_rank; i++) {
        if (recv[i * 3] != i) report('F'); // test failed
    }

    report('k'); // test successful
    return 0;
}





