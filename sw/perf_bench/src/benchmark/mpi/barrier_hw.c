#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>


int main()
{
    fgmp_info_t info = fgmp_info();
    
    fgmp_srdy(info.address);
    fgmp_ibrr(0, fgmp_addr_gen(info.width - 1,info.height - 1));
    fgmp_bbrr();

    return 0;
}


