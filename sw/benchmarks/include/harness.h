#ifndef _HARNESS_H
#define _HARNESS_H

// Benchmark harness

#include "machine.h"
#include "parop.h"

#define harness_atoi    atoi
#define harness_sin     sin
#define harness_sqrt    sqrt



void harness_print_string(const char *s)
{
    machine_putstr(s);
}


void harness_print_int32(int32_t l)
{
    if (l<0) {
        machine_putchar('-');
        l = -l;
    }

    int i;
    int32_t base = 1000000000;
    for (i=0; i<9; i++) {
        int64_t leading = l / base;
        base = base / 10;
        if (leading>0)
            machine_putchar((leading % 10) + '0');
    }
    machine_putchar((l % 10) + '0');
}


void harness_print_int64(int64_t l)
{
    if (l<0) {
        machine_putchar('-');
        l = -l;
    }

    int i;
    int64_t base = 1000000000000000000;
    for (i=0; i<18; i++) {
        int64_t leading = l / base;
        base = base / 10;
        if (leading>0)
            machine_putchar((leading % 10) + '0');
    }
    machine_putchar((l % 10) + '0');
}


void harness_print_hex32(uint32_t l)
{
    int i;
    for (i=28; i>=0; i-=4) {
        int digit = (l>>i) & 15;
        machine_putchar(digit + ((digit>9) ? ('A'-10) : ('0')));
    }
}


void harness_print_double(double f)
{
    union {
        int64_t i;
        double f;
    } x;
    x.f = f;

    if (x.i<0) machine_putchar('-');
    machine_putchar('0');
    machine_putchar('X');
    machine_putchar('1');
    machine_putchar('.');

    // ignore last 20 bits of mantissa and round
    bool guard  = (x.i & 0x100000) != 0;
    bool round  = (x.i & 0x080000) != 0;
    bool sticky = (x.i & 0x07ffff) != 0;
    int32_t inc = (round==1 && (guard==1 || sticky==1)) ? 1 : 0;
    harness_print_hex32(((x.i >> 20) & 0xffffffff) + inc);

    machine_putchar('P');
    int32_t exponent = ((x.i>>52) & 0x7ff) - 1023;
    if (exponent < 0) {
        exponent = - exponent;
        machine_putchar('-');
    } else {
        machine_putchar('+');
    }
    harness_print_int32(exponent);
}



void harness_fatal(char *s)
{
    parop_fatal(s);
}




static uint64_t xorshift128plus_next(uint64_t *seed) 
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
    a ^= a << 23;
    uint64_t c = a ^ b ^ (a >> 18) ^ (b >> 5);
    seed[0] = b;
    seed[1] = c;
    return c + b; 
}


static void xorshift128plus_jump(uint64_t *seed)
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
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
    seed[0] = c;
    seed[1] = d;
}


void harness_rand_init(unsigned long rank, uint64_t *seed)
{
    while (rank != 0) {
        xorshift128plus_jump(seed);
        rank--;
    }
}


static inline uint64_t harness_rand64(uint64_t *seed)
{
    return xorshift128plus_next(seed);
}




#endif // _HARNESS_H