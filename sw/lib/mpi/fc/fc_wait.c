#include "fc_internal.h"

// Waits for an MPI request to complete
int mpi_fc_Wait(
    MPI_Request* request,        // request
    MPI_Status* status           // status object (Status). May be MPI_STATUS_IGNORE
)
{
    int max = request->count * sizeof_mpi_datatype(request->datatype);    
    pnoo_bre();
    int len = pnoo_rcvp();
    
     if (max < len) {
        status->MPI_ERROR = MPI_UNDEFINED;
        return MPI_UNDEFINED; //NOTE: should be somethong different
    }
    
    pnoo_block_recv_no_srdy(0, len, request->buf);
    
    status->MPI_SOURCE = request->source;
    status->MPI_TAG = request->tag;
    status->MPI_ERROR = MPI_SUCCESS;
    status->len = len;
    
    return MPI_SUCCESS;
}
