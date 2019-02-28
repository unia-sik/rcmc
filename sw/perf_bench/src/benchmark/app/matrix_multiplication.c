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
    for (int i = 0; i < dimension; i++) {
        if (x == i) {
            int offset = 0;
            for (int j = 0; j < dimension; j++) {                
                if (j != i) {                    
                    fgmp_block_recv(fgmp_addr_gen(j, y), block_size * sizeof(int), A + block_size * j);
                }
            }
        } else {
            fgmp_block_send_no_srdy(fgmp_addr_gen(i, y), block_size * sizeof(int), A + block_size * x);
        }
    }
}

void send_col(int* B, int x, int y, int dimension)
{
    for (int i = 0; i < dimension; i++) {
        if (y == i) {
            for (int j = 0; j < dimension; j++) {
                if (j != i) {
                    fgmp_block_recv(fgmp_addr_gen(x, j), block_size * sizeof(int), B + block_size * j);
                }
            }
        } else {
            fgmp_block_send_no_srdy(fgmp_addr_gen(x, i), block_size * sizeof(int), B + block_size * y);
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

