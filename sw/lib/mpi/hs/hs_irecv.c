#include "hs_internal.h"


int mpi_hs_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, 
    MPI_Comm comm, MPI_Request *request)
{
    // search a free request slot
    int i;
    for (i=0; i<MAX_REQUESTS; i++) {
        if (!request_slot[i].used) break;
    }
    assert(i < MAX_REQUESTS); // none found?
    
    request_slot[i].buf = buf;
    request_slot[i].count = count;
    request_slot[i].datatype = datatype;
    request_slot[i].source = cid_from_comm(comm, source);
    request_slot[i].tag = tag;
    *request = i;
    return MPI_SUCCESS;
}
