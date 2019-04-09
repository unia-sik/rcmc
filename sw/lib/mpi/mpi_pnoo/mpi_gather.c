#include "mpi_internal.h"
#include <string.h>

#define MAP_RANK(x) (pnoo_addr_from_rank(((rank+comm->size+(x))%comm->size), comm) + comm->root)

// Gathers together values from a group of processes 
int MPI_Gather(
    const void* sendbuf,         // starting address of send buffer       
    int sendcount,               // number of elements in send buffer 
    MPI_Datatype sendtype,       // data type of send buffer elements 
    void* recvbuf,               // address of receive buffer (significant not only at root) 
    int recvcount,               // number of elements for any single receive (significant not only at root) 
    MPI_Datatype recvtype,       // data type of recv buffer elements (significant only at root)
    int root,                    // rank of receiving process 
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);

    int len = sendcount * sizeof_mpi_datatype(sendtype);    

    int rank = comm->rank;
    int relrank = rank-root;
    if (relrank<0) relrank += comm->size;

    if ((relrank & 1) == 0 && relrank < comm->size - 1) {
        fgmp_send_ready(MAP_RANK(+1));
    }
 
    memcpy(recvbuf, sendbuf, len);
    for (int i = 1; i < comm->size; i = i << 1) {
        if ((relrank & i) != 0) {
            uint64_t dest = MAP_RANK(-i);
            fgmp_send_flit(dest, len);
            mpi_transfer_send(dest, len, recvbuf);
            break;
        } else {
            if (relrank + i < comm->size) {
                uint64_t src = MAP_RANK(+i);
                fgmp_recv_wait();
                uint64_t tmp = fgmp_recv_payload();
                mpi_transfer_recv(src, tmp, recvbuf + len);
                len += tmp;
                if (relrank + (i << 1) < comm->size) {
                    fgmp_send_ready(MAP_RANK(+(i<<1)));
                }
            }
        }
    }
    return MPI_SUCCESS;
}
