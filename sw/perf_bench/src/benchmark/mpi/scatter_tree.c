#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include "random.h"

#include <stdint.h>

#define block_size 1

int main()
{        
    fgmp_info_t info = fgmp_info();
    
    int* mem_data = (int*)0x20000;

    int currentSize = block_size * info.size;
    int next = info.size >> 1;
    for (int i = info.size - 1; i != 0; i = i >> 1) {
        if ((info.rank & i) == 0) {
            int end = currentSize / 2;
            int dest = fgmp_addr_from_rank(info.rank + next, &info);
//             fgmp_bnr(dest);
            fgmp_bsf();
            
            fgmp_snd(dest, currentSize - end);
            fgmp_block_send_no_srdy(dest, (currentSize - end) * sizeof(int), mem_data + end);

            currentSize = end;
        } else if (((info.rank - next) & i) == 0) {
            fgmp_srdy(fgmp_addr_from_rank(info.rank - next, &info));
            fgmp_bre();
            currentSize = fgmp_rcvp();
            if (currentSize > 0) {
                fgmp_block_recv_no_srdy(fgmp_addr_from_rank(info.rank - next, &info), currentSize * sizeof(int), mem_data);
            }
        }

        next = next >> 1;
    }

    return 0;
}


