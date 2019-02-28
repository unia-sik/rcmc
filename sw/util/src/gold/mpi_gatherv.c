#include "mpi.h"
#include "fgmp_block.h"


// Gathers into specified locations from all processes in a group 
int MPI_Gatherv(
    const void* sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer (significant only at root) 
    const int* recvcounts,       // integer array (of length group size) containing the number of elements that are received from each process (significant only at root) 
    const int* displs,           // integer array (of length group size). Entry i specifies the displacement relative to recvbuf at which to place the incoming data from process i (significant only at root) 
    MPI_Datatype recvtype,       // data type of recv buffer elements (significant only at root) 
    int root,                    // rank of receiving process 
    MPI_Comm comm                // communicator
)
{    
    MPI_Barrier(comm);
    if (comm->rank == root) {  
        for (int i = 0; i < fgmp_addr_end(comm); i = fgmp_addr_next_by_addr(i, comm)) {
            if (i != comm->address) {                
                fgmp_srdy(i + comm->root);
                mpi_transfer_recv(i + comm->root, recvcounts[fgmp_addr_to_rank(i, comm)] * sizeof_mpi_datatype(recvtype), recvbuf + displs[fgmp_addr_to_rank(i, comm)]);
            }
        }
    } else {
        mpi_transfer_send(fgmp_addr_from_rank(root, comm) + comm->root, sendcount * sizeof_mpi_datatype(sendtype), (void*)sendbuf);
    }
    
    return MPI_SUCCESS;
}
