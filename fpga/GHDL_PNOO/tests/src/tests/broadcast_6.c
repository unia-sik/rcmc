#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#define block_size 256

int main()
{
    fgmp_info_t info = fgmp_info();
    char block[block_size];    

    for (int i = 0; i != fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
        if (info.address == i) {
            for (int j = 0; j != fgmp_addr_end(&info); j = fgmp_addr_next_by_addr(j, &info)) {
                if (info.address != j) {
                    fgmp_srdy(j);
                    fgmp_block_send(j, block_size, block);
                    fgmp_block_recv_no_srdy(j, block_size, block);
                }
            }
        } else {
            fgmp_srdy(i);
            fgmp_block_send(i, block_size, block);
            fgmp_block_recv_no_srdy(i, block_size, block);
        }
    }
       
    return 0;
}






