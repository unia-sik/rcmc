#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
      
    
    pnoo_ibrr(0, pnoo_addr_last(&info));
    pnoo_bbrr();           
    
    if (info.rank == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    pnoo_ibrr(0, pnoo_addr_last(&info));
    pnoo_bbrr();
    
    for (volatile int i = 0; i < info.rank; i++) {}
    
    pnoo_ibrr(0, pnoo_addr_last(&info));
    pnoo_bbrr();
    
    for (volatile int i = 0; i < pnoo_addr_x(info.address); i++) {}
    
    pnoo_ibrr(pnoo_addr_x(info.address), pnoo_addr_gen(pnoo_addr_x(info.address), info.height - 1));
    pnoo_bbrr();           
    
    if (pnoo_addr_y(info.address) == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    pnoo_ibrr(pnoo_addr_x(info.address), pnoo_addr_gen(pnoo_addr_x(info.address), info.height - 1));
    pnoo_bbrr();            
    
    for (volatile int i = 0; i < pnoo_addr_y(info.address); i++) {}
    
    pnoo_ibrr(pnoo_addr_x(info.address), pnoo_addr_gen(pnoo_addr_x(info.address), info.height - 1));
    pnoo_bbrr();           
    
    
    
    
    
    return 0;
}





