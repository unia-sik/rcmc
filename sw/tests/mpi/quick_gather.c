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

    uint64_t data = MPI_COMM_WORLD->rank;
    MPI_Gather(&data, 1, MPI_INT64_T, recv, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (i=0; i<max_rank; i++) {
            if (recv[i] != i) report('F'); // test failed
        }
    }
    report('k'); // test successful
    return 0;
}


