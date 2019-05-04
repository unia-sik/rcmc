#include "mpi.h"

int main(int argc,char *argv[])
{
    int max_rank;
    int rank;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    uint64_t A[128];
    uint64_t B[128];

    for (i = 0; i < 128; i++) {
        A[i] = rank;
    }

    int src = max_rank / 3;
    int dest = 0;

    if (rank == src) {
       MPI_Send(A, 128, MPI_INT64_T, dest, 0, MPI_COMM_WORLD)   ;
    } else if (rank == dest) {
        MPI_Request request;
        MPI_Status status;
        MPI_Irecv(B, 128, MPI_INT64_T, src, 0, MPI_COMM_WORLD, &request);
        for (volatile int i = 0; i < 10000; i++) {}
        MPI_Wait(&request, &status);

        int cnt = 0;
        MPI_Get_count(&status, MPI_INT64_T, &cnt);
        if (cnt != 128) {
            return -1;
        }
    }
    MPI_Finalize();
    return 0;
}





