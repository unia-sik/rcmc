#include "mpi.h"

int main() {    
    MPI_Init(0, 0);
        
    MPI_Barrier(MPI_COMM_WORLD);
    
//     for (volatile int i = 0; i < MPI_COMM_WORLD->rank; i++) {}
//         
//     MPI_Barrier(MPI_COMM_WORLD);
    
    
    MPI_Finalize();
    return 0;
}
