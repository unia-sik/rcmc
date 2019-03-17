#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t data = MPI_COMM_WORLD->rank;
    
//     int recv[MPI_COMM_WORLD->size];    
    uint64_t* recv = (uint64_t*)0x20000;
    
    int recvcounts[MPI_COMM_WORLD->size];
    int displs[MPI_COMM_WORLD->size];
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        recvcounts[i] = 1;
        displs[i] = i * 8 * 2;
    }
    
    MPI_Gatherv(&data, 1, MPI_INT64_T, recv, recvcounts, displs, MPI_INT64_T, 0, MPI_COMM_WORLD);
    
    if (MPI_COMM_WORLD->rank == 0) {
        for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
            if (recv[i * 2] != i) {
                return -1;
            }
        }
    }
       
    
    MPI_Finalize();
    return 0;
}



