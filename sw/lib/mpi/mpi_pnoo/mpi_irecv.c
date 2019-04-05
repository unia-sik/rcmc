#include "mpi_internal.h"


// Begins a nonblocking receive
int MPI_Irecv(
    void* buf,                   // initial address of receive buffer
    int count,                   // maximum number of elements in receive buffer
    MPI_Datatype datatype,       // datatype of each receive buffer element
    int source,                  // rank of source
    int tag,                     // message tag
    MPI_Comm comm,               // communicator
    MPI_Request* request         // communication request
) {
    request->buf = buf;
    request->count = count;
    request->datatype = datatype;
    request->source = source;
    request->tag = tag;
        
    pnoo_srdy(pnoo_addr_from_rank(source, comm) + comm->root);
    
    return MPI_SUCCESS;
}
