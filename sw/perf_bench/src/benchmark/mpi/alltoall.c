#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 1

int main()
{
    fgmp_info_t info = fgmp_info();

    int* local = (void*)0x20000;

    int src = info.address;
    int dest = info.address;
    for (int i = 1; i < info.size; i++) {
                
        dest = fgmp_addr_next_by_addr(dest, &info);
        
        if (dest == fgmp_addr_end(&info)) {
            dest = 0;
        }
        
        if (src == 0) {
            src = fgmp_addr_last(&info);
        } else {
            src = fgmp_addr_prev_by_addr(src, &info);
        }
        
        fgmp_srdy(src);
        
        
        fgmp_block_send_recv_no_srdy(
                dest,
                src,
                local_size * sizeof(int),
                local,
                local
        );
        
//         if (src == dest) {            
//             if (info.address < dest) {
//                 fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);                
//                 fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);
//             } else {
//                 fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);
//                 fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);
//             }
//         } else {
//             if ((info.rank & 1) == 0) {
//                 if (info.address < dest) {
//                     fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);
//                     fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);                    
//                 } else {
//                     fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);
//                     fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);                    
//                 }
//             } else {
//                 if (info.address < dest) {
//                     fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);
//                     fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);                    
//                 } else {
//                     fgmp_block_send_no_srdy(dest, local_size * sizeof(int), local);
//                     fgmp_block_recv_no_srdy(src, local_size * sizeof(int), local);                    
//                 }
//             }            
//         }
    }
    

    
    return 0;
}




