#include "pnoo.h"

#define block_size 256

int main()
{
    pnoo_info_t info = pnoo_info();
    char block[block_size];    

    for (int i = 0; i != pnoo_addr_end(&info); i = pnoo_addr_next_by_addr(i, &info)) {
        if (info.address == i) {
            for (int j = 0; j != pnoo_addr_end(&info); j = pnoo_addr_next_by_addr(j, &info)) {
                if (info.address != j) {
                    pnoo_srdy(j);
                    pnoo_block_send(j, block_size, block);
                    pnoo_block_recv_no_srdy(j, block_size, block);
                }
            }
        } else {
            pnoo_srdy(i);
            pnoo_block_send(i, block_size, block);
            pnoo_block_recv_no_srdy(i, block_size, block);
        }
    }
       
    return 0;
}





