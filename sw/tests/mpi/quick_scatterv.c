#include "mpi.h"

int main() 
{
    int rank, max_rank;

    MPI_Init(0, 0);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    uint64_t data[max_rank * 3];
    uint64_t recv;
    int sendcounts[max_rank];
    int sdispls[max_rank];

    if (rank == 0) {
        for (int i = 0; i < max_rank; i++) {
            sendcounts[i] = 1;
            sdispls[i] = i * 8 * 3;
            data[i * 3] = i;
        }
    }

    MPI_Scatterv(data, sendcounts, sdispls, MPI_INT64_T, &recv, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);

    if (recv != rank) {
        return -1;
    }
    MPI_Finalize();
    return 0;
}




