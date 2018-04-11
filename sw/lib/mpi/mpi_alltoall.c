#include "mpi_internal.h"
#include <string.h>



static inline unsigned recv_flit(cid_t cid, flit_t **ptr, unsigned *len)
{
    flit_t f = fgmp_recv_flit(cid);
    if (*len<sizeof(flit_t)) {
        unsigned l = *len;
        store_flit_fraction(*ptr, l, f);
        *len = 0;
        return l;
    } else {
        unaligned_store_flit(*ptr, f);
        (*ptr)++;
        *len -= sizeof(flit_t);
        return sizeof(flit_t);
    }
}


static inline unsigned send_flit(cid_t cid, flit_t **ptr, unsigned *len)
{
    fgmp_send_flit(cid, unaligned_load_flit(*ptr));
    (*ptr)++;
    if (*len<sizeof(flit_t)) {
        unsigned l = *len;
        *len = 0;
        return l;
    } else {
        *len -= sizeof(flit_t);
        return sizeof(flit_t);
    }
}


static void alltoall(
    MPI_Comm comm,
    flit_t *sptrs[],            // pointers to the send buffer of each core
    unsigned slens[],           // number of flits to send for each core
    flit_t *rptrs[],            // pointers to the receive buffer of each core 
                                // (will be modified)
    unsigned rlens[])           // number of flits to send for each core
{
    int group_size = comm->group.size;
    int my_rank = comm->rank;
    int next_rank = (my_rank==group_size-1) ? 0 : my_rank+1;
    int prev_rank = (my_rank==0) ? group_size-1 : my_rank-1;

    // copy own buffer
    memcpy(rptrs[my_rank], sptrs[my_rank], rlens[my_rank]);
    rlens[my_rank] = 0;
    slens[my_rank] = 0;

    unsigned rlen_total=0;
    unsigned slen_total=0;
    unsigned i;
    for (i=0; i<group_size; i++) {
        rlen_total += rlens[i];
        slen_total += slens[i];
    }

    // Virtually the processes are aranged in a ring
    // Process 0 starts a flit from process to process until it arrives at
    // process 0 again. When the flit arrives, process 0 knows that all
    // processes have entered alltoall. 

    // first direct
    if (my_rank==0) {
        slen_total -= send_flit(cid_from_comm(comm, 1), &sptrs[1], &slens[1]);
        rlen_total -= recv_flit(cid_from_comm(comm, group_size-1),
            &rptrs[group_size-1], &rlens[group_size-1]);

        // tell all processes that all are ready
        for (i=1; i<group_size; i++) {
            slen_total -= send_flit(cid_from_comm(comm, i), &sptrs[i], &slens[i]);
        }
    } else {
        rlen_total -= recv_flit(cid_from_comm(comm, prev_rank),
            &rptrs[prev_rank], &rlens[prev_rank]);
        slen_total -= send_flit(cid_from_comm(comm, next_rank),
            &sptrs[next_rank], &slens[next_rank]);

        if ((my_rank==1) && (rlens[0]==0)) {
            // Special case: data is one flit or less
            // Therefore process 1 already received all data from core 0
            // and this flit must be ignored
            fgmp_recv_flit(cid_from_comm(comm, 0)); 
        } else {
            rlen_total -= recv_flit(cid_from_comm(comm, 0), &rptrs[0], &rlens[0]);
        }
    }


    int send_rank = next_rank;
    int recv_rank = next_rank;
    while (rlen_total!=0 || slen_total!=0) {
/* code for old PIMP-1
        while ((rlens[recv_rank]!=0) && fgmp_probe(cid_from_comm(comm, recv_rank))) {
            rlen_total -= recv_flit(cid_from_comm(comm, recv_rank),
                &rptrs[recv_rank], &rlens[recv_rank]);
        }
        recv_rank++;
*/
        cid_t rcid = fgmp_any();
        if (rcid>=0) {
            int rrank = rank_in_comm(comm, rcid);
            rlen_total -= recv_flit(rcid, &rptrs[rrank], &rlens[rrank]);
        }

        if (recv_rank >= group_size) recv_rank = 0;
        while ((slens[send_rank]!=0) && !fgmp_cong()) {
            slen_total -= send_flit(cid_from_comm(comm, send_rank),
                 &sptrs[send_rank], &slens[send_rank]);
        }
        send_rank++;
        if (send_rank >= group_size) send_rank = 0;
    }
}


int MPI_Alltoall(void *sbuf, int scount, MPI_Datatype stype,
    void *rbuf, int rcount, MPI_Datatype rtype, MPI_Comm comm)
{
    size_t len = rcount * sizeof_mpi_datatype(rtype);
    assert(len ==  scount*sizeof_mpi_datatype(stype));
    int i, n=comm->group.size;
    flit_t *sptrs[n], *rptrs[n];
    unsigned slens[n], rlens[n];

    for (i=0; i<n; i++) {
        sptrs[i] = (flit_t *)(sbuf + i*len);
        slens[i] = len;
        rptrs[i] = (flit_t *)(rbuf + i*len);
        rlens[i] = len;
    }
    alltoall(comm, sptrs, slens, rptrs, rlens);
    return MPI_SUCCESS;
}


int MPI_Alltoallv(void *sbuf, int *scounts, int *sdisps, MPI_Datatype stype,
    void *rbuf, int *rcounts, int *rdisps, MPI_Datatype rtype, MPI_Comm comm)
{
    size_t slen = sizeof_mpi_datatype(stype);
    size_t rlen = sizeof_mpi_datatype(rtype);
    int i, n = comm->group.size;
    flit_t *sptrs[n], *rptrs[n];
    unsigned slens[n], rlens[n];

    for (i=0; i<n; i++) {
        sptrs[i] = (flit_t *)(sbuf + sdisps[i]*slen);
        slens[i] = scounts[i]*slen;
        rptrs[i] = (flit_t *)(rbuf + rdisps[i]*rlen);
        rlens[i] = rcounts[i]*rlen;
    }
    alltoall(comm, sptrs, slens, rptrs, rlens);
    return MPI_SUCCESS;
}
