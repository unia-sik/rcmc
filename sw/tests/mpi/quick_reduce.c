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
//    int64_t sum = 0;
    
    
    
    MPI_Reduce(&data, &min, 1, MPI_INT64_T, MPI_MIN, 0, MPI_COMM_WORLD);    
    for (volatile int i = 0; i < 100; i++) {}
//     
//     MPI_Barrier(MPI_COMM_WORLD);
//     MPI_Bcast(&min, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    MPI_Reduce(&data, &max, 1, MPI_INT64_T, MPI_MAX, 0, MPI_COMM_WORLD);
//     for (volatile int i = 0; i < 100; i++) {}
//     
//     
//     MPI_Barrier(MPI_COMM_WORLD);
//     MPI_Reduce(&data, &sum, 1, MPI_INT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        if (min != 0) {
//             || max != MPI_COMM_WORLD->size - 1 ){
            
//             || sum != sum_to_max()) {
            return -1;
        }
    }
    
    MPI_Finalize();
    return 0;
}
