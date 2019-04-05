#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
    
    if (pnoo_addr_y(info.address) == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    pnoo_ibrr(pnoo_addr_x(info.address), pnoo_addr_gen(pnoo_addr_x(info.address), info.height - 1));
    pnoo_bbrr();          
                 
    return 0;
}






