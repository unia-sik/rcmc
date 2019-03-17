#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define iterations 1
#define size 512
#define chunk 32

int main()
{
    fgmp_info_t info = fgmp_info();
    char block[size];    

    int sendCounter = 0;
    int recvCounter = 0;
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k++) {
                if (sendCounter == 0) {
                    fgmp_bnr(fgmp_addr_next(&info));
                }
                fgmp_bsf();
                fgmp_snd(fgmp_addr_next(&info), 0);
                sendCounter++;
                if (sendCounter == chunk) {
                    sendCounter = 0;
                }
            }
        }
    } else if (info.address < fgmp_addr_last(&info)) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k++) {
                if (recvCounter == 0) {
                    fgmp_srdy(fgmp_addr_prev(&info));
                }
                fgmp_bre();
                fgmp_rcvp();
                recvCounter++;
                if (recvCounter == chunk) {
                    recvCounter = 0;
                }
            }

            for (int k = 0; k < size; k++) {
                if (sendCounter == 0) {
                    fgmp_bnr(fgmp_addr_next(&info));
                }
                fgmp_bsf();
                fgmp_snd(fgmp_addr_next(&info), 0);
                sendCounter++;
                if (sendCounter == chunk) {
                    sendCounter = 0;
                }
            }
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k++) {
                if (recvCounter == 0) {
                    fgmp_srdy(fgmp_addr_prev(&info));
                }
                fgmp_bre();
                fgmp_rcvp();
                recvCounter++;
                if (recvCounter == chunk) {
                    recvCounter = 0;
                }
            }
        }
    }

    return 0;
}



