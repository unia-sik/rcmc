#include "debug.h"
#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();

    for (int i = 0; i != pnoo_addr_end(&info); i = pnoo_addr_next_by_addr(i, &info)) {
        if (info.address == i) {
            for (int j = 0; j != pnoo_addr_end(&info); j = pnoo_addr_next_by_addr(j, &info)) {
                if (info.address != j) {
                    pnoo_bnr(j);
                    pnoo_bsf();
                    pnoo_snd(j, 0xABC0 + i);
                }
            }
        } else {
            pnoo_srdy(i);
            pnoo_bre();
            uint64_t src = pnoo_rcvn();
            uint64_t msg = pnoo_rcvp();

            assert(src == i);
            assert(msg == 0xABC0 + i);
        }
    }   
           
    return 0;
}




