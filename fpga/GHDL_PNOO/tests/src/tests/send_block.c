#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"

#define block_size 512   

int main()
{
    int core_id = fgmp_core_id();       
    char block[block_size];    
    
    for (int i = 0; i < block_size; i++) {
        block[i] = i;
    }
    
    if (core_id == 0) {
        fgmp_block_send_no_srdy(0x10000, block_size, block);       
    } else if (core_id == 0x10000){
        fgmp_srdy(0);   
//         delay(100);
        fgmp_block_recv_no_srdy(0, block_size, block);
    }
    
    return 0;
}




