#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include "random.h"

#include <stdint.h>

#define local_size 32

void swap(int* A, int a, int b) {
    int tmp = A[a];
    A[a] = A[b];
    A[b] = tmp;
}

int partition_block(int* data, int n, int pivot) {
    if (n <= 0) {
        return 0;
    }

    swap(data, pivot, 0);
    int pivotValue = data[0];
    int end = 1;

    for (int i = 1; i < n; i++) {
        if (data[i] <= pivotValue) {
            swap(data, i, end);
            end++;
        }
    }

    swap(data, 0, end - 1);

    return end;
}

void local_sort(int* data, int n) {
    if (n <= 1) {
        return;
    }

    int isSorted = 1;

    for (int i = 1; i < n; i++) {
        if (data[i] < data[i - 1]) {
            isSorted = 0;
        }
    }

    if (isSorted == 1) {
        return;
    }

    int end = partition_block(data, n, n / 2);

    local_sort(data, end);
    local_sort(data + end, n - end);
}

int main()
{   
    fgmp_info_t info = fgmp_info();
        
    int* mem_data = (int*)0x20000;
    int* mem_copy = mem_data + local_size * info.size * sizeof(int);

    random_seed_t seed;
    random_init_seed_default(&seed);

    if (info.address == 0) {
        for (int i = 0; i < local_size * info.size; i++) {
            mem_data[i] = random_next_value(&seed);
        }        
    }

    int currentSize = local_size * info.size;
    int next = info.size >> 1;
    for (int i = info.size - 1; i != 0; i = i >> 1) {
        if ((info.rank & i) == 0) {
            int end = partition_block(mem_data, currentSize, currentSize / 2);
            fgmp_bsf();
            fgmp_snd(fgmp_addr_from_rank(info.rank + next, &info), currentSize - end);
            fgmp_block_send_no_srdy(fgmp_addr_from_rank(info.rank + next, &info), (currentSize - end) * sizeof(int), mem_data + end);

            currentSize = end;
        } else if (((info.rank - next) & i) == 0) {
            fgmp_srdy(fgmp_addr_from_rank(info.rank - next, &info));
            fgmp_bre();
            currentSize = fgmp_rcvp();
            if (currentSize > 0) {
                fgmp_block_recv_no_srdy(fgmp_addr_from_rank(info.rank - next, &info), currentSize * sizeof(int), mem_data);
            }
        }

        next = next >> 1;
    }

    local_sort(mem_data, currentSize);

    return 0;
}

