#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();      
    
    pnoo_ibrr(0, pnoo_addr_last(&info));
    pnoo_bbrr();           

    return 0;
}






