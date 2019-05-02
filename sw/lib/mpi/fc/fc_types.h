#pragma once
#include "pnoo.h"
#include "../common_types.h"


typedef struct {
    void* buf;                   // initial address of receive buffer
    int count;                   // maximum number of elements in receive buffer
    int source;                  // source-rank
    int tag;                     // tag
    MPI_Datatype datatype;       // datatype of each receive buffer element
} MPI_Request;
typedef pnoo_info_t mpi_communicator_t;

typedef mpi_communicator_t *MPI_Comm;

extern mpi_communicator_t mpi_comm_world;
#define MPI_COMM_WORLD (&mpi_comm_world)
#define _MPI_GROUP_WORLD (&mpi_comm_world.group)


