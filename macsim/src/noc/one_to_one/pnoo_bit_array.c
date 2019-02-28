#include "pnoo_bit_array.h"

void pnoo_bit_array_init(pnoo_bit_array_t* array)
{
    for (int i = 0; i < MAX_RANK; i++) {
        array->rdy[i] = 0;
    }
}

void pnoo_bit_array_set(pnoo_bit_array_t* array, const rank_t dest)
{
    array->rdy[dest] = 1;
}

void pnoo_bit_array_unset(pnoo_bit_array_t* array, const rank_t dest)
{
    array->rdy[dest] = 0;
}

bool pnoo_bit_array_check(const pnoo_bit_array_t* array, const rank_t dest)
{
    return array->rdy[dest] == 1;
}

