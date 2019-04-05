#include "mpi_internal.h"


int MPI_Wait(MPI_Request *request, MPI_Status *status) 
{
    // Just do a blocking receive from the formerly saved source
    mpi_request_t *r = &request_slot[*request];
    int ret = MPI_Recv(r->buf, r->count, r->datatype, r->source, r->tag, MPI_COMM_WORLD, status);
    r->used = 0;
    return ret;
}


