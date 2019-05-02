#include "hs_internal.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>


// create a new group with the given ranks
int mpi_hs_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *new)
{
    if (n==0) {
        *new = MPI_GROUP_EMPTY;
    } else {
        unsigned i;
        MPI_Group g = malloc(2*n+2); // hardcoded!
        g->size = n;
        if (group==_MPI_GROUP_WORLD) {
            for (i=0; i<n; i++) g->cids[i] = ranks[i];
        } else {
            for (i=0; i<n; i++) g->cids[i] = group->cids[ranks[i]];
        }
        *new = g;
    }
    return MPI_SUCCESS;
}


// search current process in group
int mpi_hs_Group_rank(MPI_Group group, int *rank)
{
    if (group==_MPI_GROUP_WORLD) {
        *rank = mpi_comm_world.rank;
    } else {
        int i;
        for (i=0; i<group->size; i++) {
            if (group->cids[i]==mpi_comm_world.rank) break;
        }
        *rank = (i>=group->size) ? MPI_UNDEFINED : i;
    }
    return MPI_SUCCESS;
}


// create a new communicator and copy the group list, if the current process is a member of it
int mpi_hs_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *new)
{
    assert(group!=_MPI_GROUP_WORLD && group!=MPI_GROUP_EMPTY && group!=MPI_GROUP_NULL);

    // search rank in group
    int rank;
    mpi_hs_Group_rank(group, &rank);
    if (rank==MPI_UNDEFINED) {
        *new = MPI_COMM_NULL; // this process is not in the group
        return MPI_SUCCESS;
    }

    MPI_Comm c = malloc(sizeof(mpi_communicator_t) + 2*group->size - 2); // hardcoded!
    c->rank = rank;
    memcpy(&c->group, group, 2*group->size+2); // hardcoded!
    *new = c;
    return MPI_SUCCESS;
}


// free the memory that was allocated for the group
int mpi_hs_Group_free(MPI_Group *group)
{
    assert(*group!=MPI_GROUP_EMPTY && *group!=MPI_GROUP_NULL);
    if (*group!=_MPI_GROUP_WORLD) free(*group);
    *group = MPI_GROUP_NULL;
    return MPI_SUCCESS;
}


// free the memory that was allocated for the group
int mpi_hs_Comm_free(MPI_Comm *comm)
{
    assert(*comm!=MPI_COMM_WORLD && *comm!=MPI_COMM_NULL);
    free(*comm);
    *comm = MPI_COMM_NULL;
    return MPI_SUCCESS;
}


