#include "pnoo.h"

#define iterations 1
#define size 512
#define chunk 32

int main()
{
    pnoo_info_t info = pnoo_info();
    char block[size];   
    
    for (int i = 0; i < size; i++) {
        block[i] = 0;
    }
    
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            pnoo_bnr(pnoo_addr_next(&info));
            pnoo_srdy(pnoo_addr_next(&info)); 
            for (int k = 0; k < size; k += (chunk * 8)) {
                pnoo_block_send_no_srdy(pnoo_addr_next(&info), chunk * 8, block);
                pnoo_bre();
                pnoo_rcvp();
            }
        }
    } else if (info.address < pnoo_addr_last(&info)) {
        for (int i = 0; i < iterations; i++) {
            pnoo_srdy(pnoo_addr_prev(&info));
            pnoo_bnr(pnoo_addr_prev(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                pnoo_block_recv_no_srdy(pnoo_addr_prev(&info), chunk * 8, block);
                pnoo_snd(pnoo_addr_prev(&info), 0xAC);
            }
            
            pnoo_bnr(pnoo_addr_next(&info));
            pnoo_srdy(pnoo_addr_next(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                pnoo_block_send_no_srdy(pnoo_addr_next(&info), chunk * 8, block);
                pnoo_bre();
                pnoo_rcvp();
            }
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            pnoo_srdy(pnoo_addr_prev(&info));
            pnoo_bnr(pnoo_addr_prev(&info));
            for (int k = 0; k < size; k += (chunk * 8)) {
                pnoo_block_recv_no_srdy(pnoo_addr_prev(&info), chunk * 8, block);
                pnoo_snd(pnoo_addr_prev(&info), 0xAC);
            }
        }
    }

    return 0;
}



