#include "mpi_internal.h"


int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count)
{
    *count = status->len / sizeof_mpi_datatype(datatype);
    return MPI_SUCCESS;
}

