#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();
    
    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);           
    int destination = fgmp_addr_gen((info.width - x - 1),  (info.height - y - 1));   
    
    
   if (destination != info.address) {
        for (int i = 0; i < 32; i++) {
            fgmp_snd(destination, i);
        }       
        
        for (int i = 0; i < 32; i++) {
            fgmp_srdy(destination);
            fgmp_bre();
            uint64_t src = fgmp_rcvn();
            uint64_t msg = fgmp_rcvp();
        
            assert(src == destination);
            assert(msg == i);
        }
    }   
    fgmp_brne();
    
    return 0;
}










