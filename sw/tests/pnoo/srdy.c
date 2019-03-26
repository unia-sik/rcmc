#include "pnoo.h"

int main()
{ 
    pnoo_info_t info = pnoo_info();
    
    if (info.rank != pnoo_addr_last(&info)) {
        pnoo_srdy(pnoo_addr_last(&info));
    } else {
        int end = pnoo_addr_prev_by_addr(pnoo_addr_end(&info), &info);
        for (int i = 0; i < end; i = pnoo_addr_next_by_addr(i, &info)) {
            pnoo_bnr(i);
        }
    }
    
    pnoo_brne();
    
    return 0;
}






