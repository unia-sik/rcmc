#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define size (1 << 20)
#define max_block_per_srdy 256
//32 * 8

int main()
{
    fgmp_info_t info = fgmp_info();
    int src;
    int dest;
    
    if (info.address == 0) {
        src = fgmp_addr_last(&info);
        dest = fgmp_addr_next(&info);
    } else if(info.address == fgmp_addr_last(&info)) {
        src = fgmp_addr_prev(&info);
        dest = 0;
    } else {
        src = fgmp_addr_prev(&info);
        dest = fgmp_addr_next(&info);
    }
    
    char* A = (void*)0x20000;
    char* B = A + size;
    
    
    fgmp_srdy(src);
    for (int i = 0; i < size; i += max_block_per_srdy) {
        fgmp_block_send_no_srdy(dest, max_block_per_srdy, A);
        fgmp_block_recv_no_srdy(src, max_block_per_srdy, A);
    }
    
    return 0;
}
