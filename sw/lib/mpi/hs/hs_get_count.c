#include "hs_internal.h"


int mpi_hs_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count)
{
    *count = status->len / sizeof_mpi_datatype(datatype);
    return MPI_SUCCESS;
}

