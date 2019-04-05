#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();
    
    if (info.rank == 0) {
        pnoo_bsf();
    }
    
//     if (rank == 1) {
//         set_test_result(0);
//         pnoo_bsnf();
//         assert(0);
//     }
//     
//     if (rank == 2) {
//         set_test_result(0);
//         pnoo_bre();
//         assert(0);
//     }
    
    if (info.rank == pnoo_addr_last(&info)) {
        pnoo_brne();
    }

    return 0;
}




