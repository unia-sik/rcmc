#include "mpi_internal.h"

mpi_communicator_t mpi_comm_world;

int MPI_Init(int *argc, char ***argv)
{
    mpi_comm_world = pnoo_info();
//     mpi_comm_world.group.size = mpi_comm_world.info.size;   
    
    return MPI_SUCCESS;
}
