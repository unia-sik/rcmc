#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    int data[128];    
    
    if (MPI_COMM_WORLD->rank == 0) {
        for (int i = 0; i < 128; i++) {
            data[i] = i;
        }
    } else {
        for (int i = 0; i < 128; i++) {
            data[i] = 0;
        }
    }
    
    MPI_Bcast(data, 128, MPI_INT32_T, 0, MPI_COMM_WORLD);
    
    for (int i = 0; i < 128; i++) {
        if (data[i] != i) {
            return -1;
        }
    }
    
    
    MPI_Finalize();
    return 0;
}

