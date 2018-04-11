#include "fgmp.h"
#include "mpi.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#ifdef QUIET
#define printf(...)
#endif


#define MAX 3

static int ranks[MAX] = {3, 0, 1};

int main(int argc, char *argv[])
{
    int         max_rank;
    int         comm_me, group_me, newcomm_me;
    MPI_Comm    newcomm;
    MPI_Group   group, newgroup;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_me);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    assert(max_rank>=4);

    MPI_Comm_group(MPI_COMM_WORLD, &group);
    MPI_Group_rank(group, &group_me);
    MPI_Group_incl(group, MAX, ranks, &newgroup);
    MPI_Comm_create(MPI_COMM_WORLD, newgroup, &newcomm);
    MPI_Group_free(&newgroup);

    if (newcomm!=MPI_COMM_NULL) {
        uint64_t rootrank = comm_me;
        MPI_Bcast(&rootrank, 1, MPI_INT64_T, 0, newcomm);
        MPI_Comm_rank(newcomm, &newcomm_me);
        printf("My rank is: %d in MPI_COMM_WORLD, %d in new communicator. "
            "Rank of root: %"PRId64" \n",
            comm_me, newcomm_me, rootrank);
        MPI_Barrier(newcomm);
        MPI_Comm_free(&newcomm);
    }

    MPI_Finalize();
    return 0;
}
