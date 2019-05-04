#include "debug.h"
#include "pnoo.h"

int main()
{
    int core_id = pnoo_core_id();         
    
    if (core_id == 0x10000) {
        pnoo_srdy(0);
        pnoo_bre();
        uint64_t src = pnoo_rcvn();
        uint64_t msg = pnoo_rcvp();
        
        assert(src == 0);
        assert(msg == 0xEF);
    } else if (core_id == 0) {
//         pnoo_bnr(1);
        pnoo_snd(0x10000, 0xEF);
    }
    
//     pnoo_brne();

    return 0;
}



