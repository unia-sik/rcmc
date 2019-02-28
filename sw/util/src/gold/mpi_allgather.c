#include "mpi.h"

void MPI_Allgather_send_row(void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm)
{
    int recvData[comm->width];
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);
    int offset = y * comm->width * sendcount * sizeof_mpi_datatype(sendtype);
    
    for (int i = 0; i < comm->width; i++) {
        recvData[i] = 0;
    }
    
    recvData[x] = 1;   
    
    for (int i = 1; i < comm->width; i *= 2) {
        int src = fgmp_addr_gen((x + comm->width - i) % comm->width, y) + comm->root;
        int dest = fgmp_addr_gen((x + i) % comm->width, y) + comm->root;
        fgmp_srdy(src);      
        
        if ((x & i) == 0) {

            for (int k = 0; k < comm->width; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    mpi_transfer_send(dest, sendcount * sizeof_mpi_datatype(sendtype), recvbuf + offset + sizeof_mpi_datatype(sendtype) * sendcount * k);
                }
            }
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                mpi_transfer_recv(src, sendcount * sizeof_mpi_datatype(sendtype), recvbuf + offset + sizeof_mpi_datatype(sendtype) * sendcount * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                mpi_transfer_recv(src, sendcount * sizeof_mpi_datatype(sendtype), recvbuf + offset + sizeof_mpi_datatype(sendtype) * sendcount * k);
            }

            for (int k = 0; k < comm->width; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    mpi_transfer_send(dest, sendcount * sizeof_mpi_datatype(sendtype), recvbuf + offset + sizeof_mpi_datatype(sendtype) * sendcount * k);
                }
            }
        }
    }    
}

void MPI_Allgather_send_col(void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm)
{
    int recvData[comm->height];
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);
    
    for (int i = 0; i < comm->height; i++) {
        recvData[i] = 0;
    }
    
    recvData[y] = 1;   
    int local_size = sendcount * comm->width;    
    
    for (int i = 1; i < comm->height; i *= 2) {
        int src = fgmp_addr_gen(x, (y + comm->height - i) % comm->height) + comm->root;
        int dest = fgmp_addr_gen(x, (y + i) % comm->height) + comm->root;
        fgmp_srdy(src);      
        
        if ((y & i) == 0) {
            
            for (int k = 0; k < comm->height; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    mpi_transfer_send(dest, local_size * sizeof_mpi_datatype(sendtype), recvbuf + sizeof_mpi_datatype(sendtype) * local_size * k);
                }
            }
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                mpi_transfer_recv(src, local_size * sizeof_mpi_datatype(sendtype), recvbuf + sizeof_mpi_datatype(sendtype) * local_size * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                mpi_transfer_recv(src, local_size * sizeof_mpi_datatype(sendtype), recvbuf + sizeof_mpi_datatype(sendtype) * local_size * k);
            }
            
            for (int k = 0; k < comm->height; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    mpi_transfer_send(dest, local_size * sizeof_mpi_datatype(sendtype), recvbuf + sizeof_mpi_datatype(sendtype) * local_size * k);
                }
            }
        }
    }    
}

// Gathers data from all tasks and distribute the combined data to all tasks 
int MPI_Allgather(
    const void *sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // data type of send buffer elements
    void *recvbuf,               // address of receive buffer
    int recvcount,               // number of elements received from any process
    MPI_Datatype recvtype,       // data type of receive buffer elements
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);
    int offset = comm->rank * sendcount * sizeof_mpi_datatype(sendtype);
    for (int i = 0; i < sendcount * sizeof_mpi_datatype(sendtype); i++) {
        ((char*)recvbuf)[offset + i] = ((char*)sendbuf)[i];
    }
    
    
    MPI_Allgather_send_row(recvbuf, sendcount, sendtype, comm);
    MPI_Allgather_send_col(recvbuf, sendcount, sendtype, comm);
}

