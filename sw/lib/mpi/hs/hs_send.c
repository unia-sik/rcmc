#include "hs_internal.h"


int mpi_hs_Send(const void *buf,
                int count,
                MPI_Datatype type,
                int dest,
                int tag,
                MPI_Comm comm)
{
    assert (((ptrdiff_t)buf & (sizeof(flit_t)-1)) == 0);
    int len = count*sizeof_mpi_datatype(type);
    cid_t cid = cid_from_comm(comm, dest);
    pimp2_send_flit(cid, headflit_from_tag_and_len(tag, len));
    flit_t f = pimp2_recv_flit(cid); // wait for ack
    assert (f==ACK_FLIT);
    send_raw(cid, len, buf);
    return MPI_SUCCESS;
}
