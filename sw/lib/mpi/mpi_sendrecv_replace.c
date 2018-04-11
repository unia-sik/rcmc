#include "mpi_internal.h"
#include <stdlib.h>


#define MAX_RINGBUF 4 // buffer for received flits


int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest,
    int stag, int source, int rtag, MPI_Comm comm, MPI_Status *status)
{
    assert(comm == MPI_COMM_WORLD); // FIXME: is this really necessary?
    cid_t scid = cid_from_comm(comm, dest);
    cid_t rcid = cid_from_comm(comm, source);

    int slen = count * sizeof_mpi_datatype(datatype);
    fgmp_send_flit(scid, headflit_from_tag_and_len(stag, slen));

    flit_t f = fgmp_recv_flit(rcid);
    if (scid!=rcid) {
        // no ack if send destination and receive source are the same
        fgmp_send_flit(rcid, ACK_FLIT);
        flit_t g = fgmp_recv_flit(scid);
        assert(g==ACK_FLIT);
    } else if (scid==comm->rank) return MPI_SUCCESS;

    status->MPI_SOURCE = source;
    int rlen = len_from_headflit(f);
    status->len = rlen;
    assert(rtag == tag_from_headflit(f)
        && rlen <= count*sizeof_mpi_datatype(datatype));
    status->MPI_TAG = rtag;

    flit_t *sptr = buf;
    flit_t *rptr = buf;
    flit_t ringbuf[MAX_RINGBUF];
    unsigned ringbuf_first = 0;
    unsigned ringbuf_free = MAX_RINGBUF;

    while (slen>0) {
        // send a flit whenever the send buffer is not full
        if (!fgmp_cong()) {
            fgmp_send_flit(scid, *sptr++);
            slen -= sizeof(flit_t);
            // if receiving is ahead of sendig, remove flit from ringbuffer
            if (ringbuf_free<MAX_RINGBUF) {
                sptr[-1] = ringbuf[ringbuf_first];
                ringbuf_first = (ringbuf_first + 1) % MAX_RINGBUF;
                ringbuf_free++;
            }
        }

        // don't receive last flit, if it is only partly used
        if ((rlen>=sizeof(flit_t)) && (ringbuf_free!=0) && fgmp_probe(rcid)) {
            f = fgmp_recv_flit(rcid);
            rlen -= sizeof(flit_t);
            if ((ringbuf_free<MAX_RINGBUF) || (rptr>=sptr)) {
                ringbuf[(ringbuf_first+MAX_RINGBUF-ringbuf_free) % MAX_RINGBUF]
                    = f;
                ringbuf_free--;
            } else {
                *rptr++ = f;
            }
        }
    }

    // everything sent, receive rest
    if (rlen>0) recv_raw(rcid, rlen, rptr);
    status->MPI_ERROR = MPI_SUCCESS;
    return MPI_SUCCESS;
}
