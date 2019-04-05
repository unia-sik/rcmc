#include "mpi_internal.h"
#include <string.h>

// IMPROVEMENTS :
//   * Send first flit to all processes, then overlap waiting for ack and sending

int MPI_Scatterv(const void *sbuf, const int scounts[], const int sdisps[],
    MPI_Datatype stype, void *rbuf, int rcount, MPI_Datatype rtype, int root,
    MPI_Comm comm) 
{
    int slen = sizeof_mpi_datatype(stype);
    int rlen = sizeof_mpi_datatype(rtype);

    if (comm->rank==root) {
        int i, n = comm->group.size;
        for (i=0; i<n; i++) {
            if (i!=root) {
                send_acked(cid_from_comm(comm, i), scounts[i]*slen, 
                    (flit_t *)(sbuf + sdisps[i]*slen));
            }
        }
        memcpy(rbuf, sbuf+sdisps[root]*slen, rcount*rlen);
    } else {
        recv_acked(cid_from_comm(comm, root), rcount*rlen, rbuf);
    }
    return MPI_SUCCESS;
}
