#include "mpi_internal.h"

// Sends data from all to all processes; each process may send a different amount of data and provide displacements for the input and output data.
int MPI_Alltoallv(
    const void* sendbuf,         // starting address of send buffer
    const int* sendcounts,       // integer array equal to the group size specifying the number of elements to send to each processor
    const int* sdispls,          // integer array (of length group size). Entry j specifies the displacement (relative to sendbuf from which to take the outgoing data destined for process j
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer
    const int* recvcounts,       // integer array equal to the group size specifying the maximum number of elements that can be received from each processor
    const int* rdispls,          // integer array (of length group size). Entry i specifies the displacement (relative to recvbuf at which to place the incoming data from process i
    MPI_Datatype recvtype,       // data type of receive buffer elements (handle)
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);
    
    for (int i = 0; i < sendcounts[comm->rank] * sizeof_mpi_datatype(sendtype); i++) {
        ((char*)recvbuf)[i + rdispls[comm->rank]] = ((char*)sendbuf)[i];
    }
    
    
    int src = comm->rank;
    int dest = comm->rank;
    
    for (int i = 1; i < comm->size; i++) {
        src--;
        dest++;
        
        if (dest == comm->size) {
            dest = 0;
        }
        
        if (src < 0) {
            src = comm->size - 1;
        }
        pnoo_srdy(pnoo_addr_from_rank(src, comm) + comm->root);
        if (src == dest) {            
            if (comm->rank < dest) {
                mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);                
                mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);
            } else {
                mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);
                mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);
            }
        } else {
            if ((comm->rank & 1) == 0) {
                if (comm->rank < dest) {
                    mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);
                    mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);                    
                } else {
                    mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);
                    mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);                    
                }
            } else {
                if (comm->rank < dest) {
                    mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);
                    mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);                    
                } else {
                    mpi_transfer_send(pnoo_addr_from_rank(dest, comm) + comm->root, sendcounts[dest] * sizeof_mpi_datatype(sendtype), (void*)sendbuf + sdispls[dest]);
                    mpi_transfer_recv(pnoo_addr_from_rank(src, comm) + comm->root, recvcounts[src] * sizeof_mpi_datatype(sendtype), recvbuf + rdispls[src]);                    
                }
            }
        }
    }
    return MPI_SUCCESS;
}
