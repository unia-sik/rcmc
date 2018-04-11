#include <stdio.h>
#include <string.h>
#include "fgmp.h"

int main(int argc, char *argv[])
{
    int i;
    int cid = fgmp_get_cid();
    int max_cid = fgmp_get_max_cid();
 
    if(cid == 0) {
        printf("%d: We have %d processors\n", cid, max_cid);
        for(i=1; i<max_cid; i++) {
            fgmp_send_flit(i, i);
        }
        for(i=1; i<max_cid; i++) {
            int core;
            do {
                core = fgmp_any();
            } while (core<0);
            flit_t f = fgmp_recv_flit(core);
            printf("Received %u from node %d\n", (unsigned)f, core);
        }
   } else {
        flit_t f = fgmp_recv_flit(0);
        fgmp_send_flit(0, 10*f);
   }
   return 0;
}
