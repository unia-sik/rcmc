#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();
    
    if (fgmp_addr_x(info.address) == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    fgmp_ibrr(fgmp_addr_gen(0, fgmp_addr_y(info.address)), fgmp_addr_gen(info.width - 1, fgmp_addr_y(info.address)));
    fgmp_bbrr();          
                 
    return 0;
}







