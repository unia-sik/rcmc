#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();
    
    if (info.rank == 0) {
        fgmp_bsf();
    }
    
//     if (rank == 1) {
//         set_test_result(0);
//         fgmp_bsnf();
//         assert(0);
//     }
//     
//     if (rank == 2) {
//         set_test_result(0);
//         fgmp_bre();
//         assert(0);
//     }
    
    if (info.rank == fgmp_addr_last(&info)) {
        fgmp_brne();
    }

    return 0;
}




