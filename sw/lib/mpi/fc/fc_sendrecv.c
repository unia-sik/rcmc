#include "fc_internal.h"

int mpi_fc_Sendrecv_min(int a, int b) {
    if (a < b) {
        return a;
    }
    
    return b;
}

// Sends and receives a message
int mpi_fc_Sendrecv(
    const void* sendbuf,         // initial address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // type of elements in send buffer (handle)
    int dest,                    // rank of destination
    int sendtag,                 // send tag
    void* recvbuf,               // initial address of receive buffer
    int recvcount,               // number of elements in receive buffer
    MPI_Datatype recvtype,       // type of elements in receive buffer
    int source,                  // rank of source
    int recvtag,                 // receive tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object. This refers to the receive operation.
)
{
    
    //send ready
    int recvMax = recvcount * sizeof_mpi_datatype(recvtype);
    uint32_t recvAddress = pnoo_addr_from_rank(source, comm) + comm->root;
    pnoo_srdy(recvAddress);
        
    //send-logic
    int len = sendcount * sizeof_mpi_datatype(sendtype);
    uint32_t address = pnoo_addr_from_rank(dest, comm) + comm->root;
    
    pnoo_bsf();
    pnoo_snd(address, len);
    
    //recv-logic
    pnoo_bre();
    int rlen = pnoo_rcvp();
        
    if (recvMax < rlen) {
        status->MPI_ERROR = MPI_UNDEFINED;
        return MPI_UNDEFINED; //NOTE: should be something different
    }       
        
    if (len < rlen) {
        mpi_transfer_send_recv(address, recvAddress, len, (void*)sendbuf, recvbuf);
        mpi_transfer_recv(recvAddress, rlen - len, recvbuf + len);
    } else if (len > rlen) {
        mpi_transfer_send_recv(address, recvAddress, rlen, (void*)sendbuf, recvbuf);
        mpi_transfer_send(address, len - rlen, (void*)sendbuf + rlen); //NOTE dirty hack to cast from const void* to void*
    } else {
        mpi_transfer_send_recv(address, recvAddress, len, (void*)sendbuf, recvbuf);
    }
    
   
    status->MPI_SOURCE = source;
    status->MPI_TAG = recvtag;
    status->MPI_ERROR = MPI_SUCCESS;
    status->len = rlen;
    
    
    return MPI_SUCCESS;
}
