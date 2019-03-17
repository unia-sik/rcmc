#include "mpi.h"
#include "fgmp_block.h"

void MPI_Alltoall_collect_col(const void* sendbuf, void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm) {
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);

    for (int i = 1; i < comm->height; i++) {
        int src = fgmp_addr_gen(x, ((y + comm->height - i) % comm->height)) + comm->root;
        int dest = fgmp_addr_gen(x, ((y + i) % comm->height)) + comm->root;

        fgmp_srdy(src);
        
//         if (src == dest) {            
            if (comm->address < dest) {
                mpi_transfer_send(
                    dest,
                    sendcount * comm->width * sizeof_mpi_datatype(sendtype),
                    (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
                );                
                mpi_transfer_recv(
                    src,
                    sendcount * comm->width * sizeof_mpi_datatype(sendtype),
                    recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
                );
            } else {
                mpi_transfer_recv(
                    src,
                    sendcount * comm->width * sizeof_mpi_datatype(sendtype),
                    recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
                );
                mpi_transfer_send(
                    dest,
                    sendcount * comm->width * sizeof_mpi_datatype(sendtype),
                    (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
                );
            }
//         } else {
//             if ((comm->rank & 1) == 0) {
//                 if (comm->rank < dest) {
//                     mpi_transfer_send(
//                         dest,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//                     );
//                     mpi_transfer_recv(
//                         src,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//                     );                    
//                 } else {
//                     mpi_transfer_recv(
//                         src,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//                     );
//                     mpi_transfer_send(
//                         dest,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//                     );                    
//                 }
//             } else {
//                 if (comm->rank < dest) {
//                     mpi_transfer_recv(
//                         src,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//                     );
//                     mpi_transfer_send(
//                         dest,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//                     );                    
//                 } else {
//                     mpi_transfer_send(
//                         dest,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//                     );
//                     mpi_transfer_recv(
//                         src,
//                         sendcount * comm->width * sizeof_mpi_datatype(sendtype),
//                         recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//                     );                    
//                 }
//             }            
//         }
        
        
        
//         if (y < fgmp_addr_y(dest)) {
//             fgmp_block_send_no_srdy(
//                 dest,
//                 sizeof_mpi_datatype(sendtype) * sendcount * comm->width,
//                 (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//             );
//             fgmp_block_recv_no_srdy(
//                 src,
//                 sizeof_mpi_datatype(sendtype) * sendcount * comm->width,
//                 recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//             );
//         } else {
//             fgmp_block_recv_no_srdy(
//                 src,
//                 sizeof_mpi_datatype(sendtype) * sendcount * comm->width,
//                 recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(src)
//             );
//             fgmp_block_send_no_srdy(
//                 dest,
//                 sizeof_mpi_datatype(sendtype) * sendcount * comm->width,
//                 (void*)sendbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(dest)
//             );
//         }
    }
}


void MPI_Alltoall_collect_row(const void* sendbuf, void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm) {
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);

    for (int i = 1; i < comm->width; i++) {
        int src = fgmp_addr_gen(((x + comm->width - i) % comm->width), y) + comm->root;
        int dest = fgmp_addr_gen(((x + i) % comm->width), y) + comm->root;

        fgmp_srdy(src);

        for (int j = 0; j < comm->height; j++) {
            mpi_transfer_send_recv(
                dest,
                src,
                sizeof_mpi_datatype(sendtype) * sendcount,
                recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * (comm->width * j + fgmp_addr_x(dest)),
                recvbuf+ sizeof_mpi_datatype(sendtype) * sendcount * (comm->width * j + fgmp_addr_x(dest))
            );
        }
    }
}

void MPI_Alltoall_copy_local_data(const void* sendbuf, void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm) {
    int offset = sizeof_mpi_datatype(sendtype) * sendcount * comm->width * fgmp_addr_y(comm->address);
    int n = sizeof_mpi_datatype(sendtype) * sendcount * comm->width;

    for (int i = 0; i < n; i++) {
        *((char*)recvbuf + offset + i) = *((char*)sendbuf + offset + i);
    }
}


void MPI_Alltoall_sort_recv_data_chunk(void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm, int left, int right) {
    int n = sizeof_mpi_datatype(sendtype) * sendcount;
            
    while (left < right) {
        
        for (int k = 0; k < n; k++) {
            int tmp = ((char*)recvbuf)[left * n + k];
            ((char*)recvbuf)[left * n + k] = ((char*)recvbuf)[right * n + k];
            ((char*)recvbuf)[right * n + k] = tmp;
        }
        
        
        left++;
        right--;
    }
}

void MPI_Alltoall_sort_recv_data(void* recvbuf, int sendcount, MPI_Datatype sendtype, MPI_Comm comm) {
    int zeroPos = (fgmp_addr_x(comm->rank) * 2) % comm->width;
    for (int i = 0; i < comm->height; i++) {
        MPI_Alltoall_sort_recv_data_chunk(
            recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * i,
            sendcount,
            sendtype,
            comm,
            0,
            zeroPos
        );
        
        MPI_Alltoall_sort_recv_data_chunk(
            recvbuf + sizeof_mpi_datatype(sendtype) * sendcount * comm->width * i,
            sendcount,
            sendtype,
            comm,
            zeroPos + 1,
            comm->width - 1
        ); 
    }
}

// Sends data from all to all processes
int MPI_Alltoall(
    const void* sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements to send to each process
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer
    int recvcount,               // number of elements received from any process
    MPI_Datatype recvtype,       // data type of receive buffer elements
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);
    MPI_Alltoall_collect_col(sendbuf, recvbuf, sendcount, sendtype, comm);
    MPI_Alltoall_copy_local_data(sendbuf, recvbuf, sendcount, sendtype, comm);
    MPI_Alltoall_collect_row(sendbuf, recvbuf, sendcount, sendtype, comm);
    MPI_Alltoall_sort_recv_data(recvbuf, sendcount, sendtype, comm);
}
