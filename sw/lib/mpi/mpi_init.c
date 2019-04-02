#include "mpi_internal.h"

#ifdef NEW_PIMP
// for fgmp.h
// need not be initialised, because it is cleared by the CRT anyway
flit_t fgmp_first_flit[FGMP_MAX_CID];
char fgmp_flit_buffered[FGMP_MAX_CID];
#endif

mpi_request_t request_slot[MAX_REQUESTS];
mpi_communicator_t mpi_comm_world;

int MPI_Init(int *argc, char ***argv)
{
    // for nonblocking receive
    int i;
    for (i=0; i<MAX_REQUESTS; i++)
        request_slot[i].used = 0;

    mpi_comm_world.rank = pimp2_get_cid();
    mpi_comm_world.group.size = fgmp_get_max_cid();
    mpi_comm_world.group.cids[0] = 0;
    return MPI_SUCCESS;
}
