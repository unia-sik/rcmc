#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 1

int main()
{
    fgmp_info_t info = fgmp_info();
    
    int* mem_data = (int*)0x20000;

    for (int i = 0; i < local_size; i++) {
        mem_data[i] = local_size - i; //init local data
    }

    if ((info.rank & 1) == 0 && info.rank < info.size - 1) {
        fgmp_srdy(fgmp_addr_from_rank(info.rank + 1, &info));        
    }    
    
    for (int i = 1; i != info.size; i = i << 1) {
        if ((info.rank & i) != 0) {
            fgmp_block_send_no_srdy(fgmp_addr_from_rank(info.rank - i, &info), local_size * i * sizeof(int), mem_data);

            return 0;
        } else {
            fgmp_block_recv_no_srdy(fgmp_addr_from_rank(info.rank + i, &info), local_size * i * sizeof(int), mem_data + local_size * i);
            if (info.rank + (i << 1) < info.size) {
                fgmp_srdy(fgmp_addr_from_rank(info.rank + (i << 1), &info));                
            }
        }
    }


    return 0;
}

