#include "hs_internal.h"
#include <assert.h>
#include <stdlib.h>

// split communicator in subsets with n consecutive ranks per subset
void mpi_hs_parop_comm_split_consecutive(MPI_Comm comm, unsigned n, MPI_Comm *subset)
{
    int i;
    int rank = comm->rank;
    int base_rank = (rank / n) * n;
    if (base_rank+n > comm->group.size) {
        // last subset might be smaller than n
        n = comm->group.size - base_rank;
    }

    MPI_Comm c = malloc(sizeof(mpi_communicator_t) 
        + (n-1)*sizeof(comm->group.cids[0])); // hardcoded!
      // TODO: malloc error handling
    c->rank = rank - base_rank;
    c->group.size = n;
    for (i=0; i<n; i++) c->group.cids[i] = cid_from_comm(comm, base_rank+i);
    *subset = c;
}


// helper function for parop_def_collection
MPI_Comm mpi_hs_fgmp_def_collection(int *colors)
{
    // count threads in my collection
    cid_t i, n, new_rank=0;
    cid_t max_cid = fgmp_get_max_cid();
    cid_t my_cid = pimp2_get_cid();
    int my_color = colors[my_cid];

    n=0;
    for (i=0; i<max_cid; i++) {
        if (i==my_cid) new_rank = n;
        if (colors[i] == my_color) n++;
    }

    // create new communicator
    MPI_Comm c = malloc(sizeof(mpi_communicator_t) 
        + (n-1)*sizeof(((mpi_group_t *)0)->cids[0])); // hardcoded!
      // TODO: malloc error handling
    c->rank = new_rank;
    c->group.size = n;

    // write participating thread ids
    n=0;
    for (i=0; i<max_cid; i++) {
        if (colors[i] == my_color) {
            c->group.cids[n] = i;
            n++;
        }
    }
    return c;
}


// helper function group of first n cores
MPI_Comm mpi_hs_fgmp_comm_world(int n)
{
    cid_t i;

    // create new communicator
    MPI_Comm c = malloc(sizeof(mpi_communicator_t) 
        + (n-1)*sizeof(((mpi_group_t *)0)->cids[0])); // hardcoded!
      // TODO: malloc error handling

    c->rank = pimp2_get_cid();
    c->group.size = n;
    for (i=0; i<n; i++) c->group.cids[i] = i;
    return c;
}
