#include "mpi_internal.h"

// Blocking receive for a message
int MPI_Recv(
    void* buf,                   // initial address of receive buffer
    int count,                   // maximum number of elements in receive buffer
    MPI_Datatype datatype,       // datatype of each receive buffer element
    int source,                  // rank of source
    int tag,                     // message tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object
)
{
    int max = count * sizeof_mpi_datatype(datatype);
    uint32_t address = pnoo_addr_from_rank(source, comm) + comm->root;
    pnoo_srdy(address);
    pnoo_bre();
    int len = pnoo_rcvp();
    
    if (max < len) {
        status->MPI_ERROR = MPI_UNDEFINED;
        return MPI_UNDEFINED; //NOTE: should be somethong different
    }
    
    mpi_transfer_recv(address, len, buf);
    
    status->MPI_SOURCE = source;
    status->MPI_TAG = tag;
    status->MPI_ERROR = MPI_SUCCESS;
    status->len = len;
    
    return MPI_SUCCESS;
}
