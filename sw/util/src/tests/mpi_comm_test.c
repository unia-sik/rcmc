#include "mpi.h"

int sum_to_max(MPI_Comm comm) {
    int result = 0;
    
    for (int i = 0; i < comm->size; i++) {
        result += i;
    }
    
    return result;
}

int main() {    
    MPI_Init(0, 0);

    uint64_t root = fgmp_addr_to_rank(fgmp_addr_x(MPI_COMM_WORLD->address), 0);
    mpi_communicator_t currentRow = MPI_New_Comm(
        root,
        1,
        MPI_COMM_WORLD->height,
        MPI_COMM_WORLD
    );

      
    uint64_t data = currentRow.rank;
 
    MPI_Bcast(&data, 1, MPI_INT64_T, 0, &currentRow);
    
    if (data != 0) {
        return -1;
    }
    data = currentRow.rank;
    
    uint64_t sum = 0;
    MPI_Reduce(&data, &sum, 1, MPI_INT64_T, MPI_SUM, 0, &currentRow);
    
    if (currentRow.rank == 0) {
        if (sum_to_max(&currentRow) != sum) {
            return -1;
        }
    }    
    
    uint64_t recv[MPI_COMM_WORLD->height];        
    MPI_Gather(&data, 1, MPI_INT64_T, recv, 1, MPI_INT64_T, 0, &currentRow);
    
    if (currentRow.rank == 0) {
        for (int i = 0; i < currentRow.size; i++) {
            if (recv[i] != i) {
                return -1;
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}





