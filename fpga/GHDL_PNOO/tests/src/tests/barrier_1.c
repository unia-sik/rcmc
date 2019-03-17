#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();      
    
    fgmp_ibrr(0, fgmp_addr_last(&info));
    fgmp_bbrr();           

    return 0;
}






