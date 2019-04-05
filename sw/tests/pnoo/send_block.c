#include "pnoo.h"

#define block_size 512   

int main()
{
    int core_id = pnoo_core_id();       
    char block[block_size];    
    
    for (int i = 0; i < block_size; i++) {
        block[i] = i;
    }
    
    if (core_id == 0) {
        pnoo_block_send_no_srdy(0x10000, block_size, block);       
    } else if (core_id == 0x10000){
        pnoo_srdy(0);   
//         delay(100);
        pnoo_block_recv_no_srdy(0, block_size, block);
    }
    
    return 0;
}




