#include "mpi_internal.h"
#include <string.h>


int MPI_Gather(void* sbuf, int scount, MPI_Datatype stype, 
    void* rbuf, int rcount, MPI_Datatype rtype, int root, MPI_Comm comm)
{
    unsigned len = rcount * sizeof_mpi_datatype(rtype);
    assert(len == scount * sizeof_mpi_datatype(stype));

    if (comm->rank==root) {
        cid_t i;
        cid_t n = mpi_comm_world.group.size;
        flit_t *ptrs[n];
        unsigned lens[n];
        if (comm==MPI_COMM_WORLD) {
            for (i=0; i<n; i++) {
                ptrs[i] = (flit_t *)(rbuf + i*len);
                lens[i] = len;
            }
            lens[root]=0;
        } else {
            for (i=0; i<n; i++)
                lens[i] = 0;
            n = comm->group.size;
            for (i=0; i<n; i++) {
                ptrs[comm->group.cids[i]] = (flit_t *)(rbuf + i*len);
                lens[comm->group.cids[i]] = len;
            }
            lens[comm->group.cids[root]] = 0;
        }
        gather(ptrs, lens, (n-1)*len);
        memcpy(rbuf + len*root, sbuf, len);
    } else {
        cid_t root_cid = cid_from_comm(comm, root);
        flit_t f = pimp2_recv_flit(root_cid); // wait until root is ready
        assert(f==ACK_FLIT);
        send_raw(root_cid, len, sbuf);
    }
    return MPI_SUCCESS;
}
