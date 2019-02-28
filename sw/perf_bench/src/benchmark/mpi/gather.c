#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>

#define local_size 1

int main()
{
    fgmp_info_t info = fgmp_info();
    
    int* mem = (int*)0x20000;

    if (info.address == 0) {
        int offset = 0;
        for (int i = fgmp_addr_next(&info); i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
            fgmp_block_recv(i, local_size * sizeof(int), mem + offset * local_size);
            
            offset++;
        }
    } else {   
        fgmp_block_send_no_srdy(0, local_size * sizeof(int), mem);
    }

    return 0;
}

