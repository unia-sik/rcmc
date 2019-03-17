#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>

#define local_size 1

uint64_t MSB(uint64_t d) {
    uint64_t result = (1L << 63);
    
    while (result != 0 && (result & d) == 0) {
        result = result >> 1;
    }
    
    return result;
}

int main()
{
    fgmp_info_t info = fgmp_info();
    
    uint64_t rank = info.rank;
    uint64_t bound = MSB(info.size - 1);
    int next = bound;  
    
    int* mem = (int*)0x20000;
    
    for (int i = (bound << 1) - 1; i != 0; i = i >> 1) {
        if ((rank & i) == 0 && rank + next < info.size) {
            int dest = fgmp_addr_from_rank(rank + next, &info);
            fgmp_block_send_no_srdy(dest, local_size * sizeof(int), mem);            
        } else if (((rank - next) & i) == 0) {
            int src = fgmp_addr_from_rank(rank - next, &info);
            fgmp_srdy(src);
            fgmp_block_recv_no_srdy(src, local_size * sizeof(int), mem);
        }

        next = next >> 1;
    }

    return 0;
}



