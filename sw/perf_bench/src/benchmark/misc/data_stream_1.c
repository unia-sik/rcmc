#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define iterations (1 << 5)
#define size (1 << 10)

int main()
{
    fgmp_info_t info = fgmp_info();
    char* block = (void*)0x20000;
    
    
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            fgmp_block_send_no_srdy(fgmp_addr_next(&info), size, block);
        }
    } else if (info.address < fgmp_addr_last(&info)) {
        fgmp_srdy(fgmp_addr_prev(&info));
        for (int i = 0; i < iterations; i++) {
            fgmp_block_recv_no_srdy(fgmp_addr_prev(&info), size, block);
            fgmp_block_send_no_srdy(fgmp_addr_next(&info), size, block);
        }
    } else {
        fgmp_srdy(fgmp_addr_prev(&info));
        for (int i = 0; i < iterations; i++) {
            fgmp_block_recv_no_srdy(fgmp_addr_prev(&info), size, block);
        }
    }
    
    return 0;
}


