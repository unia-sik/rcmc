#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>
#include "random.h"

#define local_size 32

void merge_block(int* data, int* copy, int n, int block_size)
{
    for (int block_start = 0; block_start < n; block_start += block_size) {
        int block0 = block_start;
        int block1 = (block_start + block_size / 2);

        for (int i = block_start; i < block_start + block_size; i++) {
            if (block0 < (block_start + block_size / 2) && block1 < block_start + block_size && block1 < n) {
                if (data[block1] < data[block0]) {
                    copy[i] = data[block1];
                    block1++;
                } else {
                    copy[i] = data[block0];
                    block0++;
                }
            } else if (block0 >= (block_start + block_size / 2) && block1 < block_start + block_size && block1 < n) {
                copy[i] = data[block1];
                block1++;
            } else if (block0 < (block_start + block_size / 2) && !(block1 < block_start + block_size && block1 < n)) {
                copy[i] = data[block0];
                block0++;
            }
        }

        for (int i = block_start; i < block_start + block_size; i++) {
            data[i] = copy[i];
        }
    }
}

void single_merge_sort(int* data, int* copy, int n)
{
    for (int block_size = 2; block_size < 2 * n; block_size *= 2) {
        merge_block(data, copy, n, block_size);
    }
}

int main()
{
    fgmp_info_t info = fgmp_info();

    int* mem_data = (int*)0x20000;
    int* mem_copy = mem_data + info.size * local_size * sizeof(int);

//     for (int i = 0; i < local_size; i++) {
//         mem_data[i] = local_size - i; //init local data
//     }
    random_seed_t seed;
    random_init_seed_default(&seed);
    for (int i = 0; i < local_size; i++) {
        mem_data[i] = random_next_value(&seed);
    }

    if ((info.rank & 1) == 0 && info.rank < info.size - 1) {
        fgmp_srdy(fgmp_addr_from_rank(info.rank + 1, &info));    
    }    
    
    single_merge_sort(mem_data, mem_copy, local_size);

    for (int i = 1; i != info.size; i = i << 1) {
        if ((info.rank & i) != 0) {
            fgmp_block_send_no_srdy(fgmp_addr_from_rank(info.rank - i, &info), local_size * i * sizeof(int), mem_data);

            return 0;
        } else {
            fgmp_block_recv_no_srdy(fgmp_addr_from_rank(info.rank + i, &info), local_size * i * sizeof(int), mem_data + local_size * i);
            if (info.rank + (i << 1) < info.size) {
                fgmp_srdy(fgmp_addr_from_rank(info.rank + (i << 1), &info));                
            }
            merge_block(mem_data, mem_copy, 2 * local_size * i, 2 * local_size * i);
        }
    }


    return 0;
}
