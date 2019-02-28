#include "pnoo_helper.h"

uint64_t min(const uint64_t a, const uint64_t b) {
    if (a < b) {
        return a;
    }
    
    return b;
}

uint64_t max(const uint64_t a, const uint64_t b) {
    if (a > b) {
        return a;
    }
    
    return b;
}

void uswap(uint64_t* a, uint64_t* b)
{
    uint64_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void swap(int64_t* a, int64_t* b)
{
    int64_t tmp = *a;
    *a = *b;
    *b = tmp;
}
