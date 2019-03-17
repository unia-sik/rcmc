#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 1

void send_row(int* A, int x, int y, int dimension)
{
    int recvData[dimension];
    
    for (int i = 0; i < dimension; i++) {
        recvData[i] = 0;
    }
    
    recvData[x] = 1;   
    
    for (int i = 1; i < dimension; i *= 2) {
        int src = fgmp_addr_gen((x + dimension - i) % dimension, y);
        int dest = fgmp_addr_gen((x + i) % dimension, y);
        fgmp_srdy(src);      
        
        if ((x & i) == 0) {
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
//                     fgmp_bnr(dest);
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, local_size * sizeof(int), A + local_size * k);
                }
            }
            for (int j = 0; j < i; j++) {
//                 fgmp_srdy(src);
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, local_size * sizeof(int), A + local_size * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
//                 fgmp_srdy(src);
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, local_size * sizeof(int), A + local_size * k);
            }
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
//                     fgmp_bnr(dest);
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, local_size * sizeof(int), A + local_size * k);
                }
            }
        }
    }    
}

void send_col(int* B, int x, int y, int dimension)
{
    int recvData[dimension];
    
    for (int i = 0; i < dimension; i++) {
        recvData[i] = 0;
    }
    
    recvData[y] = 1;    
    
    for (int i = 1; i < dimension; i *= 2) {
        int src = fgmp_addr_gen(x, ((y + dimension - i) % dimension));
        int dest = fgmp_addr_gen(x, ((y + i) % dimension));
        fgmp_srdy(src);  
        
        if ((y & i) == 0) {
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
//                     fgmp_bnr(dest);
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, local_size * sizeof(int), B + local_size * k);
                }
            }
            for (int j = 0; j < i; j++) {
//                 fgmp_srdy(src);
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, local_size * sizeof(int), B + local_size * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
//                 fgmp_srdy(src);
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, local_size * sizeof(int), B + local_size * k);
            }
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
//                     fgmp_bnr(dest);
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, local_size * sizeof(int), B + local_size * k);
                }
            }
        }
    }
}

int main()
{
    fgmp_info_t info = fgmp_info();

    int* local = (void*)0x20000;

    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);

    send_row(local, x, y, info.width);
    send_col(local, x, y, info.width);

    return 0;
}



