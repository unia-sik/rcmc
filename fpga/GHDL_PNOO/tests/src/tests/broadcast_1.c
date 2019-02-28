#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();

    for (int i = 0; i != fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
        if (info.address == i) {
            for (int j = 0; j != fgmp_addr_end(&info); j = fgmp_addr_next_by_addr(j, &info)) {
                if (info.address != j) {
                    fgmp_bnr(j);
                    fgmp_bsf();
                    fgmp_snd(j, 0xABC0 + i);
                }
            }
        } else {
            fgmp_srdy(i);
            fgmp_bre();
            uint64_t src = fgmp_rcvn();
            uint64_t msg = fgmp_rcvp();

            assert(src == i);
            assert(msg == 0xABC0 + i);
        }
    }   
           
    return 0;
}




