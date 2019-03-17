#include "mpi.h"
#include "fgmp_block.h"


// Sends and receives using a single buffer
int MPI_Sendrecv_replace(
    void* buf,                   // initial address of send and receive buffer
    int count,                   // number of elements in send and receive buffer
    MPI_Datatype datatype,       // type of elements in send and receive buffer
    int dest,                    // rank of destination 
    int sendtag,                 // send message tag
    int source,                  // rank of source 
    int recvtag,                 // receive message tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object
)
{
    //send ready
    int len = count * sizeof_mpi_datatype(datatype);
    uint32_t recvAddress = fgmp_addr_from_rank(source, comm) + comm->root;
    fgmp_srdy(recvAddress);
        
    //send-logic
    uint32_t address = fgmp_addr_from_rank(dest, comm) + comm->root;
    fgmp_bsf();
    fgmp_snd(address, len);
    
    //recv-logic
    fgmp_bre();
    fgmp_rcvp();
    
    mpi_transfer_send_recv(address, recvAddress, len, (void*)buf, buf);    
    
    status->MPI_SOURCE = source;
    status->MPI_TAG = recvtag;
    status->MPI_ERROR = MPI_SUCCESS;
    status->len = len;
    
    
    return MPI_SUCCESS;
}
