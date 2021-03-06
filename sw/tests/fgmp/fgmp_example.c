#include <stdio.h>
#include <string.h>
#include "fgmp.h"

int main(int argc, char *argv[])
{
    int i;
    int cid = fgmp_get_cid();
    int max_cid = fgmp_get_max_cid();
 
    if(cid == 0) {
        for(i=1; i<max_cid; i++) {
            fgmp_send_flit(i, i);
        }
        for(i=1; i<max_cid; i++) {
            int core;
            do {
                core = fgmp_any();
            } while (core<0);
            flit_t f = fgmp_recv_flit(core);
            if (f != 10*core) {
                putchar('F');
                putchar('\n');;
                return 'F';
            }
        }
   } else {
        flit_t f = fgmp_recv_flit(0);
        fgmp_send_flit(0, 10*f);
   }
   putchar('k');
   putchar('\n');
   return 0;
}
