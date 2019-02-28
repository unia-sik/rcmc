#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define iterations 1
#define size 512
#define chunk 32

int main()
{
    fgmp_info_t info = fgmp_info();
    char block[size];   
    
    for (int i = 0; i < size; i++) {
        block[i] = 0;
    }
    
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            fgmp_bnr(fgmp_addr_next(&info));
            fgmp_srdy(fgmp_addr_next(&info)); 
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_send_no_srdy(fgmp_addr_next(&info), chunk * 8, block);
                fgmp_bre();
                fgmp_rcvp();
            }
        }
    } else if (info.address < fgmp_addr_last(&info)) {
        for (int i = 0; i < iterations; i++) {
            fgmp_srdy(fgmp_addr_prev(&info));
            fgmp_bnr(fgmp_addr_prev(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_recv_no_srdy(fgmp_addr_prev(&info), chunk * 8, block);
                fgmp_snd(fgmp_addr_prev(&info), 0xAC);
            }
            
            fgmp_bnr(fgmp_addr_next(&info));
            fgmp_srdy(fgmp_addr_next(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_send_no_srdy(fgmp_addr_next(&info), chunk * 8, block);
                fgmp_bre();
                fgmp_rcvp();
            }
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            fgmp_srdy(fgmp_addr_prev(&info));
            fgmp_bnr(fgmp_addr_prev(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                fgmp_block_recv_no_srdy(fgmp_addr_prev(&info), chunk * 8, block);
                fgmp_snd(fgmp_addr_prev(&info), 0xAC);
            }
        }
    }

    return 0;
}



