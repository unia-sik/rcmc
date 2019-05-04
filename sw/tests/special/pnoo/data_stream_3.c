#include "pnoo.h"

#define iterations 1
#define size 512
#define chunk 32

int main()
{
    pnoo_info_t info = pnoo_info();

    int sendCounter = 0;
    int recvCounter = 0;
    if (info.address == 0) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k++) {
                if (sendCounter == 0) {
                    pnoo_bnr(pnoo_addr_next(&info));
                }
                pnoo_bsf();
                pnoo_snd(pnoo_addr_next(&info), 0);
                sendCounter++;
                if (sendCounter == chunk) {
                    sendCounter = 0;
                }
            }
        }
    } else if (info.address < pnoo_addr_last(&info)) {
        for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < size; k++) {
                if (recvCounter == 0) {
                    pnoo_srdy(pnoo_addr_prev(&info));
                }
                pnoo_bre();
                pnoo_rcvp();
                recvCounter++;
                if (recvCounter == chunk) {
                    recvCounter = 0;
                }
            }

            for (int k = 0; k < size; k++) {
                if (sendCounter == 0) {
                    pnoo_bnr(pnoo_addr_next(&info));
                }
                pnoo_bsf();
                pnoo_snd(pnoo_addr_next(&info), 0);
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
                    pnoo_srdy(pnoo_addr_prev(&info));
                }
                pnoo_bre();
                pnoo_rcvp();
                recvCounter++;
                if (recvCounter == chunk) {
                    recvCounter = 0;
                }
            }
        }
    }

    return 0;
}



