#include "mpi_internal.h"
#include <stdlib.h>


int MPI_Sendrecv(void *sbuf, int scount, MPI_Datatype stype, int dest, int stag,
    void *rbuf, int rcount, MPI_Datatype rtype, int source, int rtag,
    MPI_Comm comm, MPI_Status *status)
{
    cid_t scid = cid_from_comm(comm, dest);
    cid_t rcid = cid_from_comm(comm, source);

    int slen = scount * sizeof_mpi_datatype(stype);
    fgmp_send_flit(scid, headflit_from_tag_and_len(stag, slen));

    flit_t f = fgmp_recv_flit(rcid);
    if (scid!=rcid) {
        // no ack if send destination and receive source are the same
        fgmp_send_flit(rcid, ACK_FLIT);
        flit_t g = fgmp_recv_flit(scid);
        assert(g==ACK_FLIT);
    }

    status->MPI_SOURCE = source;
    int rlen = len_from_headflit(f);
    status->len = rlen;
    assert(rtag == tag_from_headflit(f)
        && rlen <= rcount*sizeof_mpi_datatype(rtype));
    status->MPI_TAG = rtag;

    flit_t *sptr = sbuf;
    flit_t *rptr = rbuf;
    while (slen>0) {
        // send a flit whenever the send buffer is not full
        if (!fgmp_cong()) {
            fgmp_send_flit(scid, *sptr++);
            slen -= sizeof(flit_t);
        }

        // don't receive last flit, if it is only partly used
        if ((rlen>=sizeof(flit_t)) && fgmp_probe(rcid)) {
            *rptr++ = fgmp_recv_flit(rcid);
            rlen -= sizeof(flit_t);
        }
    }

    // everything sent, receive rest
    if (rlen>0) recv_raw(rcid, rlen, rptr);
    status->MPI_ERROR = MPI_SUCCESS;
    return MPI_SUCCESS;
}
