#pragma once

#include <stdint.h>

typedef struct random_seed {
    uint64_t values[2];
} random_seed_t;

void random_init_seed_default(random_seed_t* seed);
void random_init_seed(random_seed_t* seed, uint64_t a, uint64_t b);

uint64_t random_next_value(random_seed_t *seed) ;
void random_balance(random_seed_t *seed);
