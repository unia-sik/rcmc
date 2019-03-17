#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t data = MPI_COMM_WORLD->rank;
    
//     int recv[MPI_COMM_WORLD->size];    
    uint64_t* recv = (uint64_t*)0x20000;
        
    MPI_Allgather(&data, 1, MPI_INT64_T, recv, 1, MPI_INT64_T, MPI_COMM_WORLD);
    
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        if (recv[i] != i) {
            return -1;
        }
    }
       
    
    MPI_Finalize();
    return 0;
}



