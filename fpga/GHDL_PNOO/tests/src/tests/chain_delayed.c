#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

int main()
{
    fgmp_info_t info = fgmp_info();

    if (info.address == 0) {
        for (int i = 0; i < 10; i++) {
            fgmp_bsf();
            fgmp_snd(fgmp_addr_next(&info), 0xABC0 + info.address);
            delay(100 % i);
        }
    } else {
        for (int i = 0; i < 10; i++) {
            fgmp_srdy(fgmp_addr_prev(&info));
            fgmp_bre();
            uint64_t src = fgmp_rcvn();
            uint64_t msg = fgmp_rcvp();

            assert(src == fgmp_addr_prev(&info));
            assert(msg == 0xABC0 + fgmp_addr_prev(&info));

            if (info.address < fgmp_addr_last(&info)) {
                fgmp_bsf();
                fgmp_snd(fgmp_addr_next(&info), 0xABC0 + info.address);
            }
        }
    }

    return 0;
}




