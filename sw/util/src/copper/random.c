#include "random.h"

void random_init_seed_default(random_seed_t* seed)  {    
    seed->values[0] = 0x123456789abcdef0;
    seed->values[1] = 0xfedcba9876543210;
}

void random_init_seed(random_seed_t* seed, uint64_t a, uint64_t b) {
    seed->values[0] = a;
    seed->values[1] = b;
}

uint64_t random_next_value(random_seed_t *seed) 
{
    uint64_t a = seed->values[0];
    uint64_t b = seed->values[1];
    a ^= a << 23;
    uint64_t c = a ^ b ^ (a >> 18) ^ (b >> 5);
    seed->values[0] = b;
    seed->values[1] = c;
    return c + b; 
}

void random_balance(random_seed_t *seed)
{
    uint64_t a = seed->values[0];
    uint64_t b = seed->values[1];
    uint64_t c = 0;
    uint64_t d = 0;
    uint64_t x = 0x8a5cd789635d2dff;

    for (unsigned i=0; i<128; i++) {
        if (i==64) x=0x121fd2155c472f96;
        if (x & 1) {
            c ^= a;
            d ^= b;
        }
        x >>= 1;
        uint64_t e = a ^ (a << 23);
        a = b;
        b = e ^ b ^ (e >> 18) ^ (b >> 5);
    }
    seed->values[0] = c;
    seed->values[1] = d;
}
