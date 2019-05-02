#include "hs_internal.h"


int mpi_hs_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
    MPI_Comm comm, MPI_Status *status)
{
    assert (((ptrdiff_t)buf & (sizeof(flit_t)-1)) == 0);

    cid_t c = cid_from_comm(comm, source);
    flit_t f = pimp2_recv_flit(c);
    status->MPI_SOURCE = source;
    int len = len_from_headflit(f);
    status->len = len;
    int recv_tag = tag_from_headflit(f);
    assert(recv_tag == tag);
    status->MPI_TAG = tag;
    pimp2_send_flit(c, ACK_FLIT);

    recv_raw(c, len, buf);
    status->MPI_ERROR = MPI_SUCCESS;
    return MPI_SUCCESS;
}
