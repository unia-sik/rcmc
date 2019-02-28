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

int split_block(int* data, int n, int pivot) {
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

void local_sort(int* data, int n, random_seed_t* seed) {
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

    int end = split_block(data, n, n / 2);

    local_sort(data, end, seed);
    local_sort(data + end, n - end, seed);
}

int insert_into(int* data, int currentSize, int value, int nextCore, int endCore) {
    for (int i = 0; i < currentSize; i++) {
        if (data[i] > value) {
            int tmp = data[i];
            data[i] = value;
            value = tmp;
        }
    }    
    
    if (currentSize < local_size) {
        data[currentSize] = value;
        currentSize++;
        if (nextCore < endCore) {
            fgmp_bsf();
            fgmp_snd(nextCore, 0);
        }
    } else {
        if (nextCore < endCore) {
            fgmp_bsf();
            fgmp_snd(nextCore, 1);
            fgmp_bsf();
            fgmp_snd(nextCore, value);
        }
    }
    
    return currentSize;
}

int main()
{
    fgmp_info_t info = fgmp_info();

    int* mem_data = (int*)0x20000;
    int currentSize = 0;
    
    random_seed_t seed;
    random_init_seed_default(&seed);

    if (info.address == 0) {
        for (int i = 0; i < local_size * info.size; i++) {
            currentSize = insert_into(mem_data, currentSize, random_next_value(&seed) % 16, fgmp_addr_next(&info), fgmp_addr_end(&info));
        }
    } else {        
        fgmp_srdy(fgmp_addr_prev(&info));
               
        for (int i = 0; i < local_size * info.size; i++) {
            fgmp_bre();
            if (fgmp_rcvp() == 1) {
                fgmp_bre();
                currentSize = insert_into(mem_data, currentSize, fgmp_rcvp(), fgmp_addr_next(&info), fgmp_addr_end(&info));
            } else {
                if (fgmp_addr_next(&info) < fgmp_addr_end(&info)) {
                    fgmp_bsf();
                    fgmp_snd(fgmp_addr_next(&info), 0);
                }
            }
        }
    }

    return 0;
}

