#include "fc_internal.h"

// Performs a blocking send
int mpi_fc_Send(
    const void* buf,             // initial address of send buffer
    int count,                   // number of elements in send buffer (nonnegative integer)
    MPI_Datatype datatype,       // datatype of each send buffer element (handle)
    int dest,                    // rank of destination
    int tag,                     // message tag
    MPI_Comm comm                // communicator
)
{
    int len = count * sizeof_mpi_datatype(datatype);
    uint32_t address = pnoo_addr_from_rank(dest, comm) + comm->root;
    pnoo_bsf();
    pnoo_snd(address, len);
    mpi_transfer_send(address, len, (void*)buf); //NOTE dirty hack to cast from const void* to void*
    
    return MPI_SUCCESS;
}
