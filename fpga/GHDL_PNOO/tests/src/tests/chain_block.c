#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#define block_size 256

int main()
{
    fgmp_info_t info = fgmp_info();
    
    char block[block_size];

    if (info.address == 0) {
        for (int i = 0; i < block_size; i++) {
            block[i] = (0xA5 ^ i);
        }

        fgmp_block_send(fgmp_addr_next(&info), block_size, block);
    } else {
        fgmp_block_recv(fgmp_addr_prev(&info), block_size, block);

        if (info.address < fgmp_addr_last(&info)) {
            fgmp_block_send(fgmp_addr_next(&info), block_size, block);
        } else {
            for (int i = 0; i < block_size; i++) {
                assert(block[i] == (0xA5 ^ i));
            }
        }
    }

    return 0;
}




