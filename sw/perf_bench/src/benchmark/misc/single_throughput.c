#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>

#define size (1 << 20)
//(1 << 10)
//32 * 8

int main()
{
    fgmp_info_t info = fgmp_info();
    
    char* mem = (void*)0x20000;

    if (info.address == 0) {
        fgmp_block_recv(fgmp_addr_next(&info), size, mem);
    } else if (info.address == 1){   
        fgmp_block_send_no_srdy(0, size, mem);
    }

    return 0;
}

