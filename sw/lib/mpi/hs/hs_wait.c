#include "hs_internal.h"

int mpi_hs_Recv(void *buf,
                int count,
                MPI_Datatype datatype,
                int src,
                int tag,
                MPI_Comm comm,
                MPI_Status *status);


int mpi_hs_Wait(MPI_Request *request, MPI_Status *status) 
{
    // Just do a blocking receive from the formerly saved source
    mpi_request_t *r = &request_slot[*request];
    int ret = mpi_hs_Recv(r->buf, r->count, r->datatype, r->source, r->tag, MPI_COMM_WORLD, status);
    r->used = 0;
    return ret;
}


