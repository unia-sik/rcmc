#pragma once
#include "pnoo.h"
#include "mpi.h"


static inline uint64_t sizeof_mpi_datatype(MPI_Datatype datatype)
{
    return datatype & 0xff;
}



void mpi_transfer_send(    
    int dest,                   // address of destination
    int count,                  // number of bytes in send buffer (nonnegative integer)
    const void* buf            // initial address of send buffer
);

void mpi_transfer_recv(
    int src,                    // address of source
    int count,                  // number of bytes in recv buffer (nonnegative integer)
    void* buf            // initial address of recv buffer
);

void mpi_transfer_send_recv(
    int dest,
    int src,
    int count,
    void* data_send,
    void* data_recv
);
