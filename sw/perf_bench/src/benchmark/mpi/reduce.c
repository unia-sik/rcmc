#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>

#define local_size 1

void sum(int* mem) {
    for (int i = 0; i < local_size; i++) {
        mem[i] += mem[i + local_size];
    }
}

int main()
{
    fgmp_info_t info = fgmp_info();
        
    int* mem = (int*)0x20000;
         
    if ((info.rank & 1) == 0 && info.rank < info.size - 1) {
        fgmp_srdy(fgmp_addr_from_rank(info.rank + 1, &info));        
    }  
                        
    for (int i = 1; i < info.size; i = i << 1) {
        if ((info.rank & i) != 0) {
            fgmp_block_send_no_srdy(fgmp_addr_from_rank(info.rank - i, &info), local_size * sizeof(int), mem);
            return 0;
        } else {
            if (info.rank + i < info.size) {
                fgmp_block_recv_no_srdy(fgmp_addr_from_rank(info.rank + i, &info), local_size * sizeof(int), mem + local_size * sizeof(int));
                
                if (info.rank + (i << 1) < info.size) {
                    fgmp_srdy(fgmp_addr_from_rank(info.rank + (i << 1), &info));                
                }
                
                sum(mem);
            }
        }
    }
    
    return 0;
}




