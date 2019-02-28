#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t data[MPI_COMM_WORLD->size];
    
    uint64_t recv;
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        data[i] = i;
    }
    
    MPI_Scatter(data, 1, MPI_INT64_T, &recv, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    
    if (recv != MPI_COMM_WORLD->rank) {
        return -1;
    }
       
    
    MPI_Finalize();
    return 0;
}



