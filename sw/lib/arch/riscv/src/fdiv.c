#include <stdint.h>

#define MANT_SIZE 52
#define EXP_SIZE 11
#define EXP_MASK ((1 << EXP_SIZE) - 1)
#define MANT_MASK (((uint64_t)1 << MANT_SIZE) - 1)
#define CANONICAL_NAN 0x7FF8000000000000

typedef union {
    uint64_t u;
    double f;
} uf64_t;

static inline uint32_t MULH_32(uint32_t a, uint32_t b)
{
    return ((uint64_t)a * (uint64_t)b) >> 32;
}

static inline uint64_t MULW_32(uint32_t a, uint32_t b)
{
    return (uint64_t)a * (uint64_t)b;
}

    static const uint16_t recip_1k0s[16] = {
        0xFFC4, 0xF0BE, 0xE363, 0xD76F, 0xCCAD, 0xC2F0, 0xBA16, 0xB201,
        0xAA97, 0xA3C6, 0x9D7A, 0x97A6, 0x923C, 0x8D32, 0x887E, 0x8417,
    };
    static const uint16_t recip_1k1s[16] = {
        0xF0F1, 0xD62C, 0xBFA1, 0xAC77, 0x9C0A, 0x8DDB, 0x8185, 0x76BA,
        0x6D3B, 0x64D4, 0x5D5C, 0x56B1, 0x50B6, 0x4B55, 0x4679, 0x4211,
    };

uint64_t f64_div(uint64_t a, uint64_t b)
{
    uint64_t signed_zero = (a^b) & ((uint64_t)1<<63);
    uint64_t signed_inf = signed_zero | ((uint64_t)EXP_MASK << MANT_SIZE);
    int32_t a_exp = (a >> MANT_SIZE) & EXP_MASK;
    int32_t b_exp = (b >> MANT_SIZE) & EXP_MASK;
    uint64_t a_mant = a & MANT_MASK;
    uint64_t b_mant = b & MANT_MASK;

    if (a_exp == EXP_MASK)
        return (a_mant != 0 || b_exp == EXP_MASK) ? CANONICAL_NAN : signed_inf;
    if (b_exp == EXP_MASK)
        return (b_mant != 0) ? CANONICAL_NAN : signed_zero;

    if (b_exp == 0) {
        if (b_mant == 0)
            return (a_exp == 0 && a_mant == 0) ? CANONICAL_NAN : signed_inf;
        //int shift = MANT_SIZE - ((63 - clz52(b_mant)));
        //b_exp = 1 - shift;
        //b_mant <<= shift;
        uf64_t v;
        v.u = b;
        v.f = v.f*0x1.0p64;
        b_exp = ((v.u >> MANT_SIZE) & EXP_MASK) - 64;
        b_mant = (v.u & MANT_MASK);
    }
    b_mant |= (uint64_t)1 << MANT_SIZE;

    if (a_exp == 0) {
        if (a_mant == 0)
            return signed_zero;
        //int shift = MANT_SIZE - ((63 - clz52(a_mant)));
        //a_exp = 1 - shift;
        //a_mant <<= shift;
        uf64_t v;
        v.u = a;
        v.f = v.f*0x1.0p64;
        a_exp = ((v.u >> MANT_SIZE) & EXP_MASK) - 64;
        a_mant = (v.u & MANT_MASK);
    }
    a_mant |= (uint64_t)1 << MANT_SIZE;



    int_fast64_t z_exp = a_exp - b_exp + 0x3FE;
    if (a_mant < b_mant) {
        z_exp--;
        a_mant <<= 1;
    }

    int i = (b_mant>>48) & 15;
    uint32_t r0 = (recip_1k0s[i] - ((recip_1k1s[i] * ((b_mant>>32)&0xFFFF))>>20));
    uint32_t s0 = ~(MULW_32(r0, b_mant>>21)>>7);
    //uint32_t s0 = ~((r0*b_mant)>>28);
    uint32_t r = (r0<<16) + (MULW_32(r0, s0)>>24);
    uint32_t recip32 = r + (MULH_32(r, MULH_32(s0, s0))>>16) - 2;

    uint32_t sig32Z = MULH_32(a_mant>>22, recip32);
    uint_fast64_t rem = (((a_mant - MULW_32(sig32Z, b_mant>>30))<<32)
            - MULW_32(sig32Z, b_mant<<2));

    uint32_t q = MULH_32(rem>>26, recip32) + 4;
    uint_fast64_t sigZ = ((uint_fast64_t) sig32Z<<32) + ((uint_fast64_t) q<<4);

    if ( (sigZ & 0x1C0) == 0 ) {
        sigZ &= ~(uint_fast64_t)0x7F;
        uint32_t q3 = (q>>3);
        rem = ((rem - MULW_32(q3, b_mant>>23))<<32)
                - MULW_32(q3, b_mant<<9);
        if ( rem & (uint64_t)(0x2000000000000000) ) {
            sigZ -= 0x80;
        } else {
            if ( rem ) sigZ |= 1;
        }

    }

    if (z_exp < 0) {
        if (z_exp <= -54) return signed_zero;
        // subnormal
        uint_fast64_t half = (uint_fast64_t)0x200 << (-z_exp);
        uint_fast64_t subnorm = (sigZ+half) >> (10-z_exp);
        if ((sigZ & (2*half-1))==half) subnorm &= ~1;
        return signed_zero | subnorm;
    }
    if (z_exp > 0x7FD) return signed_inf;

    uint_fast16_t roundBits = sigZ & 0x3FF;
    sigZ = (sigZ + 0x200)>>10;
    if (roundBits==0x200) sigZ &= ~1;
    return signed_zero | ((z_exp<<52) + sigZ);

}



double __divdf3(double a, double b)
{
    uf64_t aa, bb, cc;
    aa.f = a;
    bb.f = b;
    cc.u = f64_div(aa.u, bb.u);
    return cc.f;
}

