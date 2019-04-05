#include "debug.h"
#include "pnoo.h"

#define block_size 256

int main()
{
    pnoo_info_t info = pnoo_info();
    
    char block[block_size];

    if (info.address == 0) {
        for (int i = 0; i < block_size; i++) {
            block[i] = (0xA5 ^ i);
        }

        pnoo_block_send(pnoo_addr_next(&info), block_size, block);
    } else {
        pnoo_block_recv(pnoo_addr_prev(&info), block_size, block);

        if (info.address < pnoo_addr_last(&info)) {
            pnoo_block_send(pnoo_addr_next(&info), block_size, block);
        } else {
            for (int i = 0; i < block_size; i++) {
                assert(block[i] == (0xA5 ^ i));
            }
        }
    }

    return 0;
}




