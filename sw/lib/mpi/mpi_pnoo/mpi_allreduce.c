#include "mpi_internal.h"

// Combines values from all processes and distributes the result back to all processes 
int MPI_Allreduce(
    const void* sendbuf,         // starting address of send buffer
    void* recvbuf,               // starting address of receive buffer
    int count,                   // number of elements in send buffer 
    MPI_Datatype datatype,       // data type of elements of send buffer
    MPI_Op op,                   // operation
    MPI_Comm comm                // communicator
)
{
    MPI_Reduce(sendbuf, recvbuf, count, datatype, op, 0, comm);
    MPI_Barrier(comm);
    MPI_Bcast(recvbuf, count, datatype, 0, comm);

    return MPI_SUCCESS;
}
