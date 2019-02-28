#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#define num_flits 64

int main()
{
    fgmp_info_t info = fgmp_info();      

    if (info.address == 0) {
        int num_msg = 0;
        int currentCore = fgmp_addr_next(&info);          
        int recvCore[info.size];
        
        for (int i = 0; i < info.size; i++) {
            recvCore[i] = 0;
        }      
        
        fgmp_srdy(currentCore);       
       
        while(num_msg < num_flits * (info.size - 1)) {
            if (recvCore[fgmp_addr_to_rank(currentCore, &info)] < num_flits) {
                fgmp_bre();   
                uint64_t src = fgmp_rcvn();
                uint64_t msg = fgmp_rcvp();               
                
                recvCore[fgmp_addr_to_rank(src, &info)]++;  
                num_msg++;     
            }
            currentCore = fgmp_addr_next_by_addr(currentCore, &info);
            if (currentCore == fgmp_addr_end(&info)) {
                currentCore = fgmp_addr_next(&info);
            }
            
            if (recvCore[fgmp_addr_to_rank(currentCore, &info)] < num_flits) {
                fgmp_srdy(currentCore);        
            }
        }
    } else {
        for (int i = 0; i < num_flits; i++) {
            fgmp_bsf();
            fgmp_snd(0, 0xEF00 | info.address);            
        }
    }
    
    fgmp_brne();

    return 0;
}




