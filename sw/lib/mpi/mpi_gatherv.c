#include "mpi_internal.h"
#include <string.h>


int MPI_Gatherv(void* sbuf, int scount, MPI_Datatype stype, void* rbuf,
    const int rcounts[], const int rdisps[], MPI_Datatype rtype, int root, 
    MPI_Comm comm)
{
    unsigned slen = sizeof_mpi_datatype(stype);
    unsigned rlen = sizeof_mpi_datatype(rtype);

    if (comm->rank==root) {
        cid_t i;
        cid_t n = mpi_comm_world.group.size;
        flit_t *ptrs[n];
        unsigned lens[n];
        unsigned total_len = 0;
        if (comm==MPI_COMM_WORLD) {
            for (i=0; i<n; i++) {
                ptrs[i] = (flit_t *)(rbuf + rdisps[i]*rlen);
                lens[i] = rlen*rcounts[i];
                total_len += rlen*rcounts[i];
            }
            lens[root] = 0;
            total_len -= rlen*rcounts[root];
        } else {
            for (i=0; i<n; i++) {
                lens[i] = 0;
            }
            n = comm->group.size;
            for (i=0; i<n; i++) {
                if (i!=root) {
                    ptrs[comm->group.cids[i]] = (flit_t *)(rbuf + rdisps[i]*rlen);
                    lens[comm->group.cids[i]] = rlen*rcounts[i];
                    total_len += rlen*rcounts[i];
                }
             }
        }
        gather(ptrs, lens, total_len);
        memcpy(rbuf + rdisps[root]*rlen, sbuf, scount*slen);
    } else {
        cid_t root_cid = cid_from_comm(comm, root);
        flit_t f = fgmp_recv_flit(root_cid); // wait until root is ready
        assert(f==ACK_FLIT);
        send_raw(root_cid, scount*slen, sbuf);
    }
    return MPI_SUCCESS;
}
