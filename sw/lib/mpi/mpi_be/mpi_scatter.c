#include "mpi_internal.h"
#include <string.h>

// IMPROVEMENTS :
//   * Send first flit to all processes, then overlap waiting for ack and sending

int MPI_Scatter(const void *sbuf, int scount, MPI_Datatype stype, 
    void *rbuf, int rcount, MPI_Datatype rtype, int root, MPI_Comm comm) 
{
    int len = rcount * sizeof_mpi_datatype(rtype);
    assert(len == scount * sizeof_mpi_datatype(stype));

    if (comm->rank==root) {
        int i, n = comm->group.size;
        for (i=0; i<n; i++) {
            if (i!=root) {
                send_acked(cid_from_comm(comm, i), len, sbuf + i*len);
            }
        }
        memcpy(rbuf, sbuf+len*root, len);
    } else {
        recv_acked(cid_from_comm(comm, root), len, rbuf);
    }
    return MPI_SUCCESS;
}
