#include "mpi.h"

int main(int argc,char *argv[])
{
    int max_rank;
    int rank;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    uint64_t data[max_rank];
    uint64_t recv;

    for (i = 0; i < max_rank; i++) {
        data[i] = i;
    }
    MPI_Scatter(data, 1, MPI_INT64_T, &recv, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    if (recv != rank) {
        return -1;
    }
    MPI_Finalize();
    return 0;
}



