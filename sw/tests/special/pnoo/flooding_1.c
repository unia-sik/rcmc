#include "debug.h"
#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
    
    int x = pnoo_addr_x(info.address);
    int y = pnoo_addr_y(info.address);           
    int destination = pnoo_addr_gen((info.width - x - 1),  (info.height - y - 1));   
    
    
   if (destination != info.address) {
        for (int i = 0; i < 32; i++) {
            pnoo_snd(destination, i);
        }       
        
        for (int i = 0; i < 32; i++) {
            pnoo_srdy(destination);
            pnoo_bre();
            uint64_t src = pnoo_rcvn();
            uint64_t msg = pnoo_rcvp();
        
            assert(src == destination);
            assert(msg == i);
        }
    }   
    pnoo_brne();
    
    return 0;
}










