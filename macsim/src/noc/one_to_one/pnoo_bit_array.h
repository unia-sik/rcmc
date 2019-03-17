#pragma once

#include "share.h"

typedef struct {
    uint8_t rdy[MAX_RANK];
} pnoo_bit_array_t;

void pnoo_bit_array_init(pnoo_bit_array_t* array);
void pnoo_bit_array_set(pnoo_bit_array_t* array, const rank_t dest);
void pnoo_bit_array_unset(pnoo_bit_array_t* array, const rank_t dest);
bool pnoo_bit_array_check(const pnoo_bit_array_t* array, const rank_t dest);


