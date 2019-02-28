#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    uint64_t A[128];
    uint64_t B[128];
    
    for (int i = 0; i < 128; i++) {
        A[i] = MPI_COMM_WORLD->rank;
        B[i] = MPI_COMM_WORLD->rank;
    }
    
    
    int src = MPI_COMM_WORLD->size / 3;
    int dest = 0;
    
    if (MPI_COMM_WORLD->rank == src) {
        MPI_Status status;
        MPI_Sendrecv(A, 128, MPI_INT64_T, dest, 0, B, 128, MPI_INT64_T, dest, 0, MPI_COMM_WORLD, &status);
        
        for (int i = 0; i < 128; i++) {
            if (A[i] != MPI_COMM_WORLD->rank || B[i] != dest) {
                return -1;
            }
        }    
    } else if (MPI_COMM_WORLD->rank == dest) {
        MPI_Status status;
        MPI_Sendrecv(A, 128, MPI_INT64_T, src, 0, B, 128, MPI_INT64_T, src, 0, MPI_COMM_WORLD, &status);
        
        for (int i = 0; i < 128; i++) {
            if (A[i] != MPI_COMM_WORLD->rank || B[i] != src) {
                return -1;
            }
        }
    }
    

    
    
       
    
    MPI_Finalize();
    return 0;
}



