#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();
      
    
    fgmp_ibrr(0, fgmp_addr_last(&info));
    fgmp_bbrr();           
    
    if (info.rank == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    fgmp_ibrr(0, fgmp_addr_last(&info));
    fgmp_bbrr();
    
    for (volatile int i = 0; i < info.rank; i++) {}
    
    fgmp_ibrr(0, fgmp_addr_last(&info));
    fgmp_bbrr();
    
    for (volatile int i = 0; i < fgmp_addr_x(info.address); i++) {}
    
    fgmp_ibrr(fgmp_addr_x(info.address), fgmp_addr_gen(fgmp_addr_x(info.address), info.height - 1));
    fgmp_bbrr();           
    
    if (fgmp_addr_y(info.address) == 1) {
        for (volatile int i = 0; i < 10; i++) {}
    }
    
    fgmp_ibrr(fgmp_addr_x(info.address), fgmp_addr_gen(fgmp_addr_x(info.address), info.height - 1));
    fgmp_bbrr();            
    
    for (volatile int i = 0; i < fgmp_addr_y(info.address); i++) {}
    
    fgmp_ibrr(fgmp_addr_x(info.address), fgmp_addr_gen(fgmp_addr_x(info.address), info.height - 1));
    fgmp_bbrr();           
    
    
    
    
    
    return 0;
}





