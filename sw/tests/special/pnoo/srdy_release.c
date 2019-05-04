#include "pnoo.h"

#define num_flits 64

int main()
{
    pnoo_info_t info = pnoo_info();      

    if (info.address == 0) {
        int num_msg = 0;
        int currentCore = pnoo_addr_next(&info);          
        int recvCore[info.size];
        
        for (int i = 0; i < info.size; i++) {
            recvCore[i] = 0;
        }      
        
        pnoo_srdy(currentCore);       
       
        while(num_msg < num_flits * (info.size - 1)) {
            if (recvCore[pnoo_addr_to_rank(currentCore, &info)] < num_flits) {
                pnoo_bre();   
                uint64_t src = pnoo_rcvn();
                /*uint64_t msg =*/ pnoo_rcvp();               
                
                recvCore[pnoo_addr_to_rank(src, &info)]++;  
                num_msg++;     
            }
            currentCore = pnoo_addr_next_by_addr(currentCore, &info);
            if (currentCore == pnoo_addr_end(&info)) {
                currentCore = pnoo_addr_next(&info);
            }
            
            if (recvCore[pnoo_addr_to_rank(currentCore, &info)] < num_flits) {
                pnoo_srdy(currentCore);        
            }
        }
    } else {
        for (int i = 0; i < num_flits; i++) {
            pnoo_bsf();
            pnoo_snd(0, 0xEF00 | info.address);            
        }
    }
    
    pnoo_brne();

    return 0;
}




