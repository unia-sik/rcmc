#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define block_width 32
#define block_size block_width * block_width

void calc_single_block(int* A, int* B, int* C, int offset)
{
    for (int ty = 0; ty < block_width; ty++) {
        for (int tx = 0; tx < block_width; tx++) {
            int c = 0;

            for (int k = 0; k < block_width; k++) {
                int a = A[offset + ty * block_width + k];
                int b = B[offset + k * block_width + tx];
                c += a * b;
            }

            C[ty * block_width + tx] += c;
        }
    }
}

void merge_all_blocks(int* C, int dimension)
{
    for (int i = 1; i < dimension; i++) {
        for (int j = 0; j < block_size; j++) {
            C[j] += C[i * block_size + j];
            C[i * block_size + j] = 0;
        }
    }
}

void init_matrices(int* A, int* B, int* C, int x, int y, int dimension)
{    
    for (int i = 0; i < block_size; i++) {
        A[block_size * x + i] = i;
    }

    for (int i = 0; i < block_size; i++) {
        if ((i % block_width) == (i / block_width)) {
            B[block_size * y + i] = 1;
        } else {
            B[block_size * y + i] = 0;
        }
    }

    for (int i = 0; i < block_size; i++) {
        C[i] = 0;
    }
}

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
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_size * sizeof(int), A + block_size * k);
                }
            }
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_size * sizeof(int), A + block_size * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_size * sizeof(int), A + block_size * k);
            }
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_size * sizeof(int), A + block_size * k);
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
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_size * sizeof(int), B + block_size * k);
                }
            }
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_size * sizeof(int), B + block_size * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_size * sizeof(int), B + block_size * k);
            }
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_size * sizeof(int), B + block_size * k);
                }
            }
        }
    }
}

int main()
{
    fgmp_info_t info = fgmp_info();

    //place the matrices somewhere in memory
    int* A = (void*)0x20000;
    int* B = A + block_size * info.width;
    int* C = B + block_size * info.width;

    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);

    init_matrices(A, B, C, x, y, info.width);
    send_row(A, x, y, info.width);
    send_col(B, x, y, info.width);

    for (int i = 0; i < info.width; i++) {
        calc_single_block(A, B, C, i * block_size);
    }
    
    return 0;
}


