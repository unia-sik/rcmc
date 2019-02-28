#include "fgmp.h"
#include "fgmp_block.h"
#include "debug.h"
#include "fgmp_addr.h"

#define local_size 1

int main()
{
    fgmp_info_t info = fgmp_info();
        
    int* local = (int*)0x20000;       
    
    int offset = 0;
    for (int i = 0; i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
        if (info.address == i) {
            for (int j = 0; j < fgmp_addr_end(&info); j = fgmp_addr_next_by_addr(j, &info)) {
                if (i != j) {
                    fgmp_block_send_no_srdy(j, local_size * sizeof(int), local + local_size * offset);
                }
            }
        } else {
            fgmp_block_recv(i, local_size * sizeof(int), local + local_size * offset);
        }
        
        offset++;
    }
        
    return 0;
}



