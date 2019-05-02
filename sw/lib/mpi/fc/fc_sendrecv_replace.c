#include "fc_internal.h"


// Sends and receives using a single buffer
int mpi_fc_Sendrecv_replace(
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
    uint32_t recvAddress = pnoo_addr_from_rank(source, comm) + comm->root;
    pnoo_srdy(recvAddress);
        
    //send-logic
    uint32_t address = pnoo_addr_from_rank(dest, comm) + comm->root;
    pnoo_bsf();
    pnoo_snd(address, len);
    
    //recv-logic
    pnoo_bre();
    pnoo_rcvp();
    
    mpi_transfer_send_recv(address, recvAddress, len, (void*)buf, buf);    
    
    status->MPI_SOURCE = source;
    status->MPI_TAG = recvtag;
    status->MPI_ERROR = MPI_SUCCESS;
    status->len = len;
    
    
    return MPI_SUCCESS;
}
