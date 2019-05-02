#ifndef _HS_TYPES_H
#define _HS_TYPES_H

#include "../common_types.h"

typedef int MPI_Request;
typedef struct {
    int         rank;
    mpi_group_t group;
}  mpi_communicator_t;

typedef mpi_communicator_t *MPI_Comm;

extern mpi_communicator_t mpi_comm_world;
#define MPI_COMM_WORLD (&mpi_comm_world)
#define _MPI_GROUP_WORLD (&mpi_comm_world.group)

#endif

