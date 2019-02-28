#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"

int main()
{
    int core_id = fgmp_core_id();         
    
    if (core_id == 0x10000) {
        fgmp_srdy(0);
        fgmp_bre();
        uint64_t src = fgmp_rcvn();
        uint64_t msg = fgmp_rcvp();
        
        assert(src == 0);
        assert(msg == 0xEF);
    } else if (core_id == 0) {
//         fgmp_bnr(1);
        fgmp_snd(0x10000, 0xEF);
    }
    
//     fgmp_brne();

    return 0;
}



