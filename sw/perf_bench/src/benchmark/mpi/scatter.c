#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define block_size 1

int main()
{
    fgmp_info_t info = fgmp_info();

    //place the data somewhere in memory
    int* A = (void*)0x20000;

    if (info.address == 0) {
        int offset = 0;
        for (int i = fgmp_addr_next(&info); i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
            fgmp_block_send_no_srdy(i, block_size * sizeof(int), A + offset * block_size);
            offset++;
        }
    } else {
        fgmp_block_recv(0, block_size * sizeof(int), A);
    }

    return 0;
}


