#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define block_size 1

void init_block(int* A, int num_core)
{
    for (int i = 0; i < block_size * num_core; i++) {
        A[i] = 1;
    }
}

void work(int* A)
{
    int sum = 0;

    for (int i = 0; i < block_size; i++) {
        sum += A[i];
    }

    A[0] = sum;
}

void merge_work(int* A, int num_core)
{
    int sum = 0;

    for (int i = 0; i < num_core; i++) {
        sum += A[i * block_size];
    }

    A[0] = sum;
}

int main()
{
    fgmp_info_t info = fgmp_info();

    //place the data somewhere in memory
    int* A = (void*)0x20000;

    if (info.address == 0) {
        init_block(A, info.size);

        int offset = 0;
        for (int i = fgmp_addr_next(&info); i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
            fgmp_block_send_no_srdy(i, block_size * sizeof(int), A + offset * block_size);
            
            offset++;
        }

        work(A);

        offset = 0;
        for (int i = fgmp_addr_next(&info); i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
            fgmp_block_recv(i, block_size * sizeof(int), A + offset * block_size);
            
            offset++;
        }

        merge_work(A, info.size);
    } else {
        fgmp_block_recv(0, block_size * sizeof(int), A);
        work(A);
        fgmp_block_send_no_srdy(0, block_size * sizeof(int), A);
    }

    return 0;
}

