#include "fgmp.h"
#include "fgmp_block.h"


void fgmp_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv) {
    fgmp_srdy(src);
    fgmp_bnr(dest);
    fgmp_block_send_recv_no_srdy(dest, src, n, data_send, data_recv);
}

void fgmp_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv) {    
    
//     if (fgmp_core_id() < dest) {        
//         fgmp_block_send_no_srdy(dest, n, data_send);
//         fgmp_block_recv_no_srdy(src, n, data_recv); 
//     } else {
//         fgmp_block_recv_no_srdy(src, n, data_recv); 
//         fgmp_block_send_no_srdy(dest, n, data_send);
//     }
//     
//     return;
    while (n != 0) {
        if (n >= 256) {
//             fgmp_srdy(src);
//             fgmp_bnr(dest);
            fgmp_block_send_no_srdy(dest, 256, data_send);
            fgmp_block_recv_no_srdy(src, 256, data_recv);
            
            n -= 256;
            data_send += 256;
            data_recv += 256;
        } else {
//             fgmp_srdy(src);
//             fgmp_bnr(dest);
            fgmp_block_send_no_srdy(dest, n, data_send);
            fgmp_block_recv_no_srdy(src, n, data_recv);           
            
            n = 0;
        }
    }    
}
