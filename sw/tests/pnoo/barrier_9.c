#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
    
    for (volatile int i = 0; i < pnoo_addr_x(info.address); i++) {}
    
    pnoo_ibrr(pnoo_addr_gen(0, pnoo_addr_y(info.address)), pnoo_addr_gen(info.width - 1, pnoo_addr_y(info.address)));
    pnoo_bbrr();           
    
    return 0;
}






