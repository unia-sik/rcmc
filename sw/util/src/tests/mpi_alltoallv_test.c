#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t data[MPI_COMM_WORLD->size * 2];
//     uint64_t recv[MPI_COMM_WORLD->size];
    
    uint64_t* recv = (uint64_t*)0x20000;
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        data[i * 2] = MPI_COMM_WORLD->rank;
    }    
    
    int sendcounts[MPI_COMM_WORLD->size];
    int sdispls[MPI_COMM_WORLD->size];
    int recvcounts[MPI_COMM_WORLD->size];
    int rdispls[MPI_COMM_WORLD->size];
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        sendcounts[i] = 1;
        sdispls[i] = i * 8 * 2;
        recvcounts[i] = 1;
        rdispls[i] = i * 8 * 3;
    }
    
    MPI_Alltoallv(data, sendcounts, sdispls, MPI_INT64_T, recv, recvcounts, rdispls, MPI_INT64_T, MPI_COMM_WORLD);
    
    
    for (int i = 0; i < MPI_COMM_WORLD->size; i++) {
        if (recv[i * 3] != i) {
            return -1;
        }
    }  

    MPI_Finalize();
    return 0;
}





