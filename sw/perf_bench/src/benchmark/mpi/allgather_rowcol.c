#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 1

void send_row(int* A, int x, int y, int dimension)
{
    for (int i = 0; i < dimension; i++) {
        if (x == i) {
            for (int j = 0; j < dimension; j++) {
                if (j != i) {
                    fgmp_block_recv(fgmp_addr_gen(j, y), local_size * sizeof(int), A + local_size * j);
                }
            }
        } else {
            fgmp_block_send_no_srdy(fgmp_addr_gen(i, y), local_size * sizeof(int), A + local_size * x);
        }
    }
}

void send_col(int* B, int x, int y, int dimension)
{
    for (int i = 0; i < dimension; i++) {
        if (y == i) {
            for (int j = 0; j < dimension; j++) {
                if (j != i) {
                    fgmp_block_recv(fgmp_addr_gen(x, j), local_size * sizeof(int), B + local_size * j);
                }
            }
        } else {
            fgmp_block_send_no_srdy(fgmp_addr_gen(x, i), local_size * sizeof(int), B + local_size * y);
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


