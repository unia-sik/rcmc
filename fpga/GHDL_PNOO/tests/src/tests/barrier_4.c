#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();
          
    fgmp_ibrr(fgmp_addr_x(info.address), fgmp_addr_gen(fgmp_addr_x(info.address), info.height - 1));
    fgmp_bbrr();           

    return 0;
}





