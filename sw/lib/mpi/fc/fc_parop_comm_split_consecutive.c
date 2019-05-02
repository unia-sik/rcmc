#include "fc_internal.h"
#include <assert.h>
#include <stdlib.h>

// split communicator in subsets with n consecutive ranks per subset
void mpi_fc_parop_comm_split_consecutive(MPI_Comm comm, unsigned n, MPI_Comm *subset)
{
}


// helper function for parop_def_collection
MPI_Comm mpi_fc_fgmp_def_collection(int *colors)
{
    return MPI_COMM_WORLD;
}


// helper function group of first n cores
MPI_Comm mpi_fc_fgmp_comm_world(int n)
{
    return MPI_COMM_WORLD;
}
