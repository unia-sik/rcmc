#include "mpi_internal.h"


int MPI_Bcast(void* buf, int count, MPI_Datatype datatype, int root,
    MPI_Comm comm)
{
    unsigned len = count * sizeof_mpi_datatype(datatype);

    if (comm->rank==root) {
        int i;
        int n = comm->group.size;
        flit_t *ptr = buf;
        flit_t f = *ptr++;
        for (i=0; i<n; i++) {
            if (i!=root)
                fgmp_send_flit(cid_from_comm(comm, i), f);
        }

        wait_for_ack(comm);

        while (len>sizeof(flit_t)) { // one flit less, was already sent!
            f = *ptr++;
            len -=  sizeof(flit_t);
            for (i=0; i<n; i++) {
                if (i!=root)
                    fgmp_send_flit(cid_from_comm(comm, i), f);
            }
        }
    } else {
        recv_acked(cid_from_comm(comm, root), len, (flit_t *)buf);
    }
    return MPI_SUCCESS;
}
