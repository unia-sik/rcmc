#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{ 
    fgmp_info_t info = fgmp_info();
    
    if (info.rank != fgmp_addr_last(&info)) {
        fgmp_srdy(fgmp_addr_last(&info));
    } else {
        int end = fgmp_addr_prev_by_addr(fgmp_addr_end(&info), &info);
        for (int i = 0; i < end; i = fgmp_addr_next_by_addr(i, &info)) {
            fgmp_bnr(i);
        }
    }
    
    fgmp_brne();
    
    return 0;
}






