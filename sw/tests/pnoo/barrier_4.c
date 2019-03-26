#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
          
    pnoo_ibrr(pnoo_addr_x(info.address), pnoo_addr_gen(pnoo_addr_x(info.address), info.height - 1));
    pnoo_bbrr();           

    return 0;
}






