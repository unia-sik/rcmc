#include "mpi.h"
#include "fgmp_block.h"

int MPI_Bcast_fix_rank(int rank, int root, int size) {
    int result = rank - root;
    if (result < 0) {
        result += size;
    }
    
    return result;
}

uint64_t MPI_Bcast_MSB(uint64_t d) {
    uint64_t result = (1L << 63);
    
    while (result != 0 && (result & d) == 0) {
        result = result >> 1;
    }
    
    return result;
}

// Broadcasts a message from the process with rank "root" to all other processes of the communicator
int MPI_Bcast(
    void* buffer,                // starting address of buffer
    int count,                   // number of entries in buffer
    MPI_Datatype datatype,       // data type of buffer
    int root,                    // rank of broadcast root
    MPI_Comm comm                // communicator
)
{    
    MPI_Barrier(comm);
    uint64_t rank = MPI_Bcast_fix_rank(comm->rank, root, comm->size);
    uint64_t bound = MPI_Bcast_MSB(comm->size - 1);
    int next = bound;  
    
    for (int i = (bound << 1) - 1; i != 0; i = i >> 1) {
        if ((rank & i) == 0 && rank + next < comm->size) {
            int dest = fgmp_addr_from_rank(rank + next, comm) + comm->root;
            mpi_transfer_send(dest, count * sizeof_mpi_datatype(datatype), buffer);
        } else if (((rank - next) & i) == 0) {
            int src = fgmp_addr_from_rank(rank - next, comm) + comm->root;
            fgmp_srdy(src);
            mpi_transfer_recv(src, count * sizeof_mpi_datatype(datatype), buffer);
        }

        next = next >> 1;
    }
}
