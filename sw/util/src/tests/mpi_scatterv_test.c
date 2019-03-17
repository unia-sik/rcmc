#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t data[MPI_COMM_WORLD->size * 3];
    
    uint64_t recv;
    
    int sendcounts[MPI_COMM_WORLD->size];
    int sdispls[MPI_COMM_WORLD->size];
        
    if (MPI_COMM_WORLD->rank == 0) {
        for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
            sendcounts[i] = 1;
            sdispls[i] = i * 8 * 3;
            data[i * 3] = i;
        }
    }
    
    MPI_Scatterv(data, sendcounts, sdispls, MPI_INT64_T, &recv, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    
    if (recv != MPI_COMM_WORLD->rank) {
        return -1;
    }
       
    
    MPI_Finalize();
    return 0;
}




