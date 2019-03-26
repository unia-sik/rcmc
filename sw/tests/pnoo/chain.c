#include "debug.h"
#include "pnoo.h"

int main()
{
    pnoo_info_t info = pnoo_info();

    if (info.address == 0) {
        pnoo_bnr(pnoo_addr_next(&info));
        pnoo_snd(pnoo_addr_next(&info), 0xABC0 + info.address);
    } else {
        pnoo_srdy(pnoo_addr_prev(&info));
        pnoo_bre();
        uint64_t src = pnoo_rcvn();
        uint64_t msg = pnoo_rcvp();

        assert(src == pnoo_addr_prev(&info));
        assert(msg == 0xABC0 + pnoo_addr_prev(&info));

        if (info.address < pnoo_addr_last(&info)) {
            pnoo_snd(pnoo_addr_next(&info), 0xABC0 + info.address);
        }
    }

    return 0;
}



