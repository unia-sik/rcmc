#include "hs_internal.h"
#include <string.h>

int mpi_hs_Bcast(void* buf, int count, MPI_Datatype datatype, int root,
    MPI_Comm comm);


int mpi_hs_Gather(const void* sbuf, int scount, MPI_Datatype stype, 
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



int mpi_hs_Allgather(const void *send_buf,
                     int send_count,
                     MPI_Datatype send_datatype,
                     void *receive_buf,
                     int receive_count,
                     MPI_Datatype receive_datatype,
                     MPI_Comm comm)
{
    mpi_hs_Gather(send_buf, send_count, send_datatype,
        receive_buf, receive_count, receive_datatype,
        0, comm);
    return mpi_hs_Bcast(receive_buf, receive_count, receive_datatype,
        0, comm);
}
