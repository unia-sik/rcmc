#include "mpi_internal.h"

void mpi_transfer_send(    
    int dest,                   // address of destination
    int count,                  // number of bytes in send buffer (nonnegative integer)
    const void* buf            // initial address of send buffer
)
{
    pnoo_block_send_no_srdy(dest, count, (void*)buf);
}

void mpi_transfer_recv(
    int src,                    // address of source
    int count,                  // number of bytes in recv buffer (nonnegative integer)
    const void* buf            // initial address of recv buffer
)
{
    uint64_t offset = count & 0x7;
    
    if (offset == 0) {
        //is aligned
        pnoo_block_recv_no_srdy(src, count, (void*)buf);
    } else {
        //not aligned
        uint64_t overrideBuffer[2]; //2 buffer, because the override could to protect the buffer from override
        overrideBuffer[1] = ((uint64_t*)buf)[count / 8];
        uint64_t overrideMask = -1;
        overrideMask = overrideMask >> (offset * 8);
        pnoo_block_recv_no_srdy(src, count, (void*)buf);
        ((uint64_t*)buf)[(count / 8)] = 
            (((uint64_t*)buf)[(count / 8)] & (overrideMask)) |
            (overrideBuffer[1] & (~overrideMask));
    }
    
}

void mpi_transfer_send_recv(
    int dest,
    int src,
    int count,
    void* data_send,
    void* data_recv
)
{
    uint64_t offset = count & 0x7;
    
    if (offset == 0) {
        //is aligned
        pnoo_block_send_recv_no_srdy(dest, src, count, data_send, data_recv);
    } else {
        //not aligned
        uint64_t overrideBuffer[2]; //2 buffer, because the override could to protect the buffer from override
        overrideBuffer[1] = ((uint64_t*)data_recv)[count / 8];
        uint64_t overrideMask = -1;
        overrideMask = overrideMask >> (offset * 8);
        pnoo_block_send_recv_no_srdy(dest, src, count, data_send, data_recv);
        ((uint64_t*)data_recv)[(count / 8)] = 
            (((uint64_t*)data_recv)[(count / 8)] & (overrideMask)) |
            (overrideBuffer[1] & (~overrideMask));
    }
}


// #include "mpi_transfer.h"
// #include "fgmp.h"
// #include "pnoo_block.h"
// #include "stdint.h"
// 
// uint64_t mpi_transfer_load(const void* buf, MPI_Datatype datatype) {
//     int size = sizeof_mpi_datatype(datatype);
//         
//     switch (size) {
//         case 1: return *((uint8_t*)buf);
//         case 2: return *((uint16_t*)buf);
//         case 4: return *((uint32_t*)buf);
//         case 8: return *((uint64_t*)buf);
//     }
// }
// 
// void mpi_transfer_store(const void* buf, MPI_Datatype datatype, uint64_t d) {
//     int size = sizeof_mpi_datatype(datatype);
//         
//     switch (size) {
//         case 1: *((uint8_t*)buf) = d; return;
//         case 2: *((uint16_t*)buf) = d; return;
//         case 4: *((uint32_t*)buf) = d; return;
//         case 8: *((uint64_t*)buf) = d; return;
//     }
// }
// 
// void mpi_transfer_send(
//     const void* buf,             // initial address of send buffer
//     int count,                   // number of elements in send buffer (nonnegative integer)
//     MPI_Datatype datatype,       // datatype of each send buffer element (handle)
//     int dest                     // rank of destination
// )
// {
//     if (count == 1) {
//         pnoo_bsf();
//         pnoo_snd(dest, mpi_transfer_load(buf, datatype));
//     } else if(sizeof_mpi_datatype(datatype) == 4 && count == 2) {                
//         uint32_t* d = (uint32_t*)buf;
//         pnoo_bsf();
//         pnoo_snd(dest, (((uint64_t)d[0]) << 32) | d[1]);
//     } else if(sizeof_mpi_datatype(datatype) == 2 && count == 4) {                
//         uint16_t* d = (uint16_t*)buf;
//         pnoo_bsf();
//         pnoo_snd(dest, (((uint64_t)d[0]) << 48) | (((uint64_t)d[1]) << 32) | (((uint64_t)d[2]) << 16) | d[3]);
//     } else if(sizeof_mpi_datatype(datatype) == 1 && count == 8) {                
//         uint8_t* d = (uint8_t*)buf;
//         pnoo_bsf();
//         pnoo_snd(
//             dest,
//                 (((uint64_t)d[0]) << 56) | 
//                 (((uint64_t)d[1]) << 48) | 
//                 (((uint64_t)d[2]) << 40) | 
//                 (((uint64_t)d[3]) << 32) | 
//                 (((uint64_t)d[4]) << 24) | 
//                 (((uint64_t)d[5]) << 16) | 
//                 (((uint64_t)d[6]) << 8) | 
//                 d[7]
//             );
//     } else if(sizeof_mpi_datatype(datatype) == 8) {
//         pnoo_block_send_no_srdy(dest, count * sizeof_mpi_datatype(datatype), (void*)buf);
//     } else {
//         pnoo_bsf();
//         uint64_t offset = ((uint64_t)buf) & 0x7;
//         pnoo_snd(dest, offset);
//         pnoo_block_send_no_srdy(dest, count * sizeof_mpi_datatype(datatype) + offset, (void*)buf - offset);
//     }
// }
// 
// void mpi_transfer_recv(
//     void* buf,                   // initial address of receive buffer
//     int count,                   // maximum number of elements in receive buffer
//     MPI_Datatype datatype,       // datatype of each receive buffer element
//     int source                   // rank of source
// )
// {
//     if (count == 1) {
//         pnoo_bre();
//         uint64_t d = pnoo_rcvp();
//         mpi_transfer_store(buf, datatype, d);        
//     } else if(sizeof_mpi_datatype(datatype) == 4 && count == 2) {                
//         pnoo_bre();
//         uint64_t d = pnoo_rcvp();
//         ((uint32_t*)buf)[0] = ((uint32_t*)d)[1];
//         ((uint32_t*)buf)[1] = ((uint32_t*)d)[0];
//     } else if(sizeof_mpi_datatype(datatype) == 2 && count == 4) {     
//         pnoo_bre();
//         uint64_t d = pnoo_rcvp();           
//         ((uint16_t*)buf)[0] = ((uint16_t*)d)[3];
//         ((uint16_t*)buf)[1] = ((uint16_t*)d)[2];
//         ((uint16_t*)buf)[2] = ((uint16_t*)d)[1];
//         ((uint16_t*)buf)[3] = ((uint16_t*)d)[0];
//     } else if(sizeof_mpi_datatype(datatype) == 1 && count == 8) {   
//         pnoo_bre();
//         uint64_t d = pnoo_rcvp();       
//         ((uint8_t*)buf)[0] = ((uint8_t*)d)[7];
//         ((uint8_t*)buf)[1] = ((uint8_t*)d)[6];
//         ((uint8_t*)buf)[2] = ((uint8_t*)d)[5];
//         ((uint8_t*)buf)[3] = ((uint8_t*)d)[4]; 
//         ((uint8_t*)buf)[4] = ((uint8_t*)d)[3];
//         ((uint8_t*)buf)[5] = ((uint8_t*)d)[2];
//         ((uint8_t*)buf)[6] = ((uint8_t*)d)[1];
//         ((uint8_t*)buf)[7] = ((uint8_t*)d)[0];        
//     } else if(sizeof_mpi_datatype(datatype) == 8) {
//         pnoo_block_recv_no_srdy(source, count * sizeof_mpi_datatype(datatype), buf);
//     } else if(sizeof_mpi_datatype(datatype) == 4) {
//         pnoo_bre();
//         uint64_t offset = pnoo_rcvp();
//         
//         pnoo_block_recv_no_srdy(source, count * sizeof_mpi_datatype(datatype), buf);
//     } else {
//         
//     }
// }
// 
// 
