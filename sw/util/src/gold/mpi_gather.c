#include "mpi.h"
#include "fgmp_block.h"


int MPI_Gather_fix_rank(int rank, int root, int size) {
    int result = rank - root;
    if (result < 0) {
        result += size;
    }
    
    return result;
}

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
    
    int local_size = sendcount * sizeof_mpi_datatype(sendtype);    
    
    if ((MPI_Gather_fix_rank(comm->rank, root, comm->size) & 1) == 0 && MPI_Gather_fix_rank(comm->rank, root, comm->size) < comm->size - 1) {
        fgmp_srdy(fgmp_addr_from_rank(MPI_Gather_fix_rank(comm->rank, root, comm->size) + 1, comm) + comm->root);        
    }    
     
    for (int i = 1; i < comm->size; i = i << 1) {
        if ((MPI_Gather_fix_rank(comm->rank, root, comm->size) & i) != 0) {
            uint64_t dest = fgmp_addr_from_rank(MPI_Gather_fix_rank(comm->rank, root, comm->size) - i, comm) + comm->root;
            
            fgmp_bsf();
            fgmp_snd(dest, local_size);
            
            if (i == 1) {
                mpi_transfer_send(dest, local_size, (void*)sendbuf);
            } else {
                mpi_transfer_send(dest, local_size, recvbuf);
            }

            return MPI_SUCCESS;
        } else {
            if (i == 1) {
                for (int k = 0; k < local_size; k++) {
                    ((char*)recvbuf)[k] = ((char*)sendbuf)[k];
                }
            }
            
            if (MPI_Gather_fix_rank(comm->rank, root, comm->size) + i < comm->size) {
                uint64_t src = fgmp_addr_from_rank(MPI_Gather_fix_rank(comm->rank, root, comm->size) + i, comm) + comm->root;
                
                fgmp_bre();
                uint64_t tmp = fgmp_rcvp();
                mpi_transfer_recv(src, tmp, recvbuf + local_size);
                local_size += tmp;
                
                if (MPI_Gather_fix_rank(comm->rank, root, comm->size) + (i << 1) < comm->size) {
                    fgmp_srdy(fgmp_addr_from_rank(MPI_Gather_fix_rank(comm->rank, root, comm->size) + (i << 1), comm) + comm->root);                
                }
            }
        }
    }
    
    return MPI_SUCCESS;
}
