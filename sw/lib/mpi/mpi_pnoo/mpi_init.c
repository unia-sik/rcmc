#include "mpi_internal.h"

#ifndef OLD_PIMP2
// for fgmp.h
// need not be initialised, because it is cleared by the CRT anyway
flit_t fgmp_first_flit[FGMP_MAX_CID];
char fgmp_flit_buffered[FGMP_MAX_CID];
#endif

mpi_communicator_t mpi_comm_world;

int MPI_Init(int *argc, char ***argv)
{
    mpi_comm_world = pnoo_info();
//     mpi_comm_world.group.size = mpi_comm_world.info.size;   
    
    return MPI_SUCCESS;
}
