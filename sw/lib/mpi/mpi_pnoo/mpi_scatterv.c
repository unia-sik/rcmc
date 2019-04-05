#include "mpi_internal.h"

// Sends data from one process to all other processes in a communicator 
int MPI_Scatterv(
    const void* sendbuf,         // address of send buffer (significant only at root) 
    const int* sendcounts,       // integer array (of length group size) specifying the number of elements to send to each processor 
    const int* sdispls,          // integer array (of length group size). Entry i specifies the displacement (relative to sendbuf from which to take the outgoing data to process i
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer
    int recvcount,               // number of elements in receive buffer
    MPI_Datatype recvtype,       // data type of receive buffer elements
    int root,                    // rank of sending process
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);
    if (comm->rank == root) {        
        for (int i = 0; i < pnoo_addr_end(comm); i = pnoo_addr_next_by_addr(i, comm)) {
            if (i != comm->address) {                
                uint64_t r = pnoo_addr_to_rank(i, comm);
                mpi_transfer_send(i + comm->root, sendcounts[r] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[r]);
            }
        }
    } else {        
        pnoo_srdy(pnoo_addr_from_rank(root, comm) + comm->root);
        mpi_transfer_recv(pnoo_addr_from_rank(root, comm) + comm->root, recvcount * sizeof_mpi_datatype(recvtype), recvbuf);
    }
    
    return MPI_SUCCESS;
}
