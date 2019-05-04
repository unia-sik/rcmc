#include "mpi.h"

int main()
{
    MPI_Init(0, 0);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
