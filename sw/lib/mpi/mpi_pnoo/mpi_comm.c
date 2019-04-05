#include "mpi_internal.h"

mpi_communicator_t MPI_New_Comm(
    const uint32_t root,         // The root of the new rectangle (root = lower left corner)
    const uint32_t width,        // width of the rectangle
    const uint32_t height,       // height of the rectangle
    MPI_Comm comm                // current communicator
) {
    return pnoo_info_virtual(pnoo_addr_from_rank(root, comm) + comm->root, width, height);
}
