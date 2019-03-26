#include "pnoo.h"

#define iterations 1
#define size 512

int main()
{
    pnoo_info_t info = pnoo_info();
    char block[size];    
    
    for (int i = 0; i < size; i++) {
        block[i] = 0;
    }
    
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            pnoo_block_send_no_srdy(pnoo_addr_next(&info), size, block);
        }
    } else if (info.address < pnoo_addr_last(&info)) {
        pnoo_srdy(pnoo_addr_prev(&info));
        for (int i = 0; i < iterations; i++) {
            pnoo_block_recv_no_srdy(pnoo_addr_prev(&info), size, block);
            pnoo_block_send_no_srdy(pnoo_addr_next(&info), size, block);
        }
    } else {
        pnoo_srdy(pnoo_addr_prev(&info));
        for (int i = 0; i < iterations; i++) {
            pnoo_block_recv_no_srdy(pnoo_addr_prev(&info), size, block);
        }
    }
    
    return 0;
}


