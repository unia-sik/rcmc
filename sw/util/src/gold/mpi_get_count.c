#include "mpi.h"


// Gets the number of "top level" elements
int MPI_Get_count(
    const MPI_Status* status,    // return status of receive operation (Status)
    MPI_Datatype datatype,       // datatype of each receive buffer element (handle)
    int* count                   // number of received elements (integer)
)
{
    *count = status->len / sizeof_mpi_datatype(datatype);
    return MPI_SUCCESS;
}

