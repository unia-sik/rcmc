#include "mpi.h"

int max_rank;

int sum_to_max() {
    int result = 0;
    
    for (int i = 0; i < max_rank; i++) {
        result += i;
    }
    
    return result;
}


int main() 
{
    int rank;
    MPI_Init(0, 0);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    uint64_t data = rank;

    int64_t min = 0;
    int64_t max = 0;
    int64_t sum = 0;

    MPI_Allreduce(&data, &min, 1, MPI_INT64_T, MPI_MIN, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Allreduce(&data, &max, 1, MPI_INT64_T, MPI_MAX, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Allreduce(&data, &sum, 1, MPI_INT64_T, MPI_SUM, MPI_COMM_WORLD);
    
    if (min != 0 || (max != max_rank-1) || sum != sum_to_max()) {
        return -1;
    }
    
    MPI_Finalize();
    return 0;
}
