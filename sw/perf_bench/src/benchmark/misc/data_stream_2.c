#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define iterations (1 << 5)
#define size (1 << 10)
#define chunk 32

int main()
{
    fgmp_info_t info = fgmp_info();
    char* block = (void*)0x20000;
    
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_send(fgmp_addr_next(&info), chunk * 8, block);                
            }
        }
    } else if (info.address < fgmp_addr_last(&info)) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_recv(fgmp_addr_prev(&info), chunk * 8, block);
            }
            
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_send(fgmp_addr_next(&info), chunk * 8, block);
            }
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_recv(fgmp_addr_prev(&info), chunk * 8, block);
            }
        }
    }   

    return 0;
}



