#include "fc_internal.h"

mpi_communicator_t MPI_New_Comm(
    const uint32_t root,         // The root of the new rectangle (root = lower left corner)
    const uint32_t width,        // width of the rectangle
    const uint32_t height,       // height of the rectangle
    MPI_Comm comm                // current communicator
) {
    return pnoo_info_virtual(pnoo_addr_from_rank(root, comm) + comm->root, width, height);
}




// TODO: implementation!

int mpi_fc_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *new)
{
    return MPI_SUCCESS;
}
int mpi_fc_Group_rank(MPI_Group group, int *rank)
{
    return MPI_SUCCESS;
}

int mpi_fc_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *new)
{
    return MPI_SUCCESS;
}

int mpi_fc_Group_free(MPI_Group *group)
{
    return MPI_SUCCESS;
}

int mpi_fc_Comm_free(MPI_Comm *comm)
{
    return MPI_SUCCESS;
}


