/**
 * armv3.c
 * ARM v3 ISA
 *
 * MacSim project
 *
 *
 * Restrictions:
 *   - not yet implemented, equal to armv6m.c
 *   - timing not correct
 *
 */

#include "armv3.h"
#include "memory.h"
#include <inttypes.h>
#include <string.h>


#define ARMV3_MODE_HANDLER	1
#define ARMV3_MODE_THREAD	2

// latency of instructions (in clock cycles)
#define ARMV3_LATENCY_ARITH             1
#define ARMV3_LATENCY_SHIFT             2
#define ARMV3_LATENCY_LOAD              2 // + memory latency
#define ARMV3_LATENCY_STORE             1 // + memory latency
#define ARMV3_LATENCY_MUL               4 // can be less
#define ARMV3_LATENCY_MLA               5 // can be less
#define ARMV3_LATENCY_MULL              5 // can be less
#define ARMV3_LATENCY_MLAL              6 // can be less


#define ARMV3_LATENCY_IJUMP             3
#define ARMV3_LATENCY_JUMP              3
#define ARMV3_LATENCY_CALL              3
#define ARMV3_LATENCY_SYS               3
#define ARMV3_LATENCY_WRITEPC           2       // additonal



static inline uint32_t instr_clz32(uint32_t v)
{
    uint32_t count;
    for (count = 0; count < 32; count++)
    {
        if ((v & 0x80000000) != 0) break;
        v = v << 1;
    }
    return count;
}


static inline void instr_set_apsr_nz(node_t *node, bool setcond, uint32_t result)
{
    if (setcond) {
        node->core.armv3.apsr_n = (result >> 31)&1;
        node->core.armv3.apsr_z = (result & 0xffffffff) == 0 ? 1 : 0;
    }
}


static inline uint_fast32_t instr_lsl(node_t *node, bool setcond, uint32_t value, uint32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value << shift;
        if (setcond) node->core.armv3.apsr_c = (value >> (32-shift)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        result = 0;
        if (setcond) node->core.armv3.apsr_c = (shift==32) ? (value & 1) : 0;
    }

    instr_set_apsr_nz(node, setcond, result);
    return result;
}

static inline uint_fast32_t instr_lsr(node_t *node, bool setcond, uint32_t value, uint32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value >> shift; // unsigned
        if (setcond) node->core.armv3.apsr_c = (value >> (shift-1)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        result = 0;
        if (setcond) node->core.armv3.apsr_c = (shift==32) ? ((value >> 31) & 1): 0;
    }

    instr_set_apsr_nz(node, setcond, result);
    return result;
}

static inline uint_fast32_t instr_asr(node_t *node, bool setcond, int32_t value, int32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value >> shift; // signed
        if (setcond) node->core.armv3.apsr_c = (value >> (shift-1)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        if (setcond) node->core.armv3.apsr_c = (value >> 31) & 1;
        result = 0 - ((value >> 21) & 1);
    }

    instr_set_apsr_nz(node, setcond, result);
    return result;
}

static inline uint_fast32_t instr_ror(node_t *node, bool setcond, uint32_t value, uint_fast32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else
    {
        result = (value >> shift) | (value << (32-shift));
        if (setcond) node->core.armv3.apsr_c = (result >> 31) & 1;
    }

    instr_set_apsr_nz(node, setcond, result);
    return result;
}

static inline uint_fast32_t instr_add_with_carry(node_t *node, bool setcond, uint32_t a, uint32_t b, uint_fast64_t carry)
// side effect: set PSR flags
{
    uint_fast64_t r = (uint_fast64_t)a + (uint_fast64_t)b + carry;
    if (setcond) {
        instr_set_apsr_nz(node, setcond, r);
        node->core.armv3.apsr_c = (r >> 32)&1;
        node->core.armv3.apsr_v = ((a >> 31)^(r >> 31))&((b >> 31)^(r >> 31)) & 1;
    }
    return r;
}



#define BRu(h,l)	(((iw)>>(l))&((1<<((h)-(l)+1))-1))
#define BRs(h,l)	((((int32_t)(iw))<<(31-(h)))>>(31-(h)+(l)))

#define REG_Au		node->core.armv3.reg[BRu(2, 0)]
#define REG_Bu		node->core.armv3.reg[BRu(5, 3)]
#define REG_Cu		node->core.armv3.reg[BRu(8, 6)]
#define REG_Du		node->core.armv3.reg[BRu(10, 8)]
#define REG_As		((int32_t)node->core.armv3.reg[BRu(2, 0)])
#define REG_Bs		((int32_t)node->core.armv3.reg[BRu(5, 3)])

#define IMM_3		BRu(8, 6)
#define IMM_5		BRu(10, 6)
#define IMM_7		BRu(6, 0)
#define IMM_8		BRu(7, 0)


// new:
#define REG_LR		node->core.armv3.reg[14]
#define REG_SP		node->core.armv3.reg[13]

#define REGm            node->core.armv3.reg[BRu(3, 0)]
#define REGs            node->core.armv3.reg[BRu(11, 8)]
#define REGd            node->core.armv3.reg[BRu(15, 12)]
#define REGdODD         node->core.armv3.reg[BRu(15, 12) | 1]
#define REGn            node->core.armv3.reg[BRu(19, 16)]

#define REGmS           ((int_fast64_t)(int32_t)REGm)
#define REGsS           ((int_fast64_t)(int32_t)REGs)
#define REGdS           ((int_fast64_t)(int32_t)REGd)
#define REGnS           ((int_fast64_t)(int32_t)REGn)
#define REGmU           ((uint_fast64_t)(uint32_t)REGm)
#define REGsU           ((uint_fast64_t)(uint32_t)REGs)
#define REGdU           ((uint_fast64_t)(uint32_t)REGd)
#define REGnU           ((uint_fast64_t)(uint32_t)REGn)

#define FIELD_REGd      BRu(15, 12)


static inline instruction_class_t ic_load(node_t *node, uint_fast32_t iw, 
        unsigned access_type, addr_t addr)
{
    uint64_t reg;
    instruction_class_t latency = memory_load_u64_direct(node, access_type, addr, &reg);
    REGd = reg;
    if ((iw&0x0000f000)==0x000f000) { // write to pc
        node->nextpc = reg;
        return ARMV3_LATENCY_LOAD + ARMV3_LATENCY_WRITEPC + latency;
    }
    return ARMV3_LATENCY_LOAD + latency;
}

static inline instruction_class_t ic_store(node_t *node, uint32_t iw,
    unsigned access_type, addr_t addr)
{
    return ARMV3_LATENCY_STORE + memory_store_direct(node, access_type, addr, REGd);
}

static inline instruction_class_t ic_mull(node_t *node, uint_fast32_t iw, uint_fast64_t result)
{
    if (iw&0x00100000) { // S bit
        node->core.armv3.apsr_n = (result >> 63)&1;
        node->core.armv3.apsr_z = (result & 0xffffffffffffffff) == 0 ? 1 : 0;
    }
    REGd = result & 0xffffffff;
    REGn = (result >> 32) & 0xffffffff;
    return ARMV3_LATENCY_MULL;
}

uint32_t instr_shifted_reg(node_t *node, bool setcond, uint_fast32_t iw)
{
    switch (iw&0xf0) {
        case 0x00:
        case 0x80:  return instr_lsl(node, setcond, REGm, BRu(11, 7));
        case 0x20:
        case 0xa0:  return instr_lsr(node, setcond, REGm, BRu(11, 7));
        case 0x40:
        case 0xc0:  return instr_asr(node, setcond, REGm, BRu(11, 7));
        case 0x60:
        case 0xe0:  if ((iw&0x0ff0)==0x00000060) { // rrx if ror #0
                        uint32_t value = REGm;
                        uint32_t msb = node->core.armv3.apsr_c ? 0x80000000 : 0;
                        if (setcond) node->core.armv3.apsr_c = value & 1;
                        return (value >> 1) | msb;
                    }
                    return instr_ror(node, setcond, REGm, BRu(11, 7));
        case 0x10:  return instr_lsl(node, setcond, REGm, REGs);
        case 0x30:  return instr_lsr(node, setcond, REGm, REGs);
        case 0x50:  return instr_asr(node, setcond, REGm, REGs);
        case 0x70:  return instr_ror(node, setcond, REGm, REGs);
    }
    fatal("Invalid shift in instruction 0x%08x at 0x%08x\n", iw, node->pc);
}


// ---------------------------------------------------------------------
// Main switch for decoding and execution
// ---------------------------------------------------------------------


instruction_class_t armv3_execute_iw(node_t *node, uint_fast32_t iw)
{
    unsigned cond = (iw>>28) & 15;
    bool enable;
    switch (cond) {
        case 0x00:  enable = node->core.armv3.apsr_z; break;   // eq
        case 0x01:  enable = !node->core.armv3.apsr_z; break;  // ne
        case 0x02:  enable = node->core.armv3.apsr_c; break;   // cs
        case 0x03:  enable = !node->core.armv3.apsr_c; break;  // cc
        case 0x04:  enable = node->core.armv3.apsr_n; break;   // mi
        case 0x05:  enable = !node->core.armv3.apsr_n; break;  // pl
        case 0x06:  enable = node->core.armv3.apsr_v; break;   // vs
        case 0x07:  enable = !node->core.armv3.apsr_v; break;  // vc

        case 0x08:  enable = node->core.armv3.apsr_c &&
                             !node->core.armv3.apsr_z; break;   // hi
        case 0x09:  enable = !node->core.armv3.apsr_c ||
                             node->core.armv3.apsr_z; break;    // lo
        case 0x0a:  enable = node->core.armv3.apsr_n ==
                             node->core.armv3.apsr_v; break;    // ge
        case 0x0b:  enable = node->core.armv3.apsr_n !=
                             node->core.armv3.apsr_v; break;    // lt
        case 0x0c:  enable = !node->core.armv3.apsr_z && 
                             (node->core.armv3.apsr_n == 
                             node->core.armv3.apsr_v); break;   // gt
        case 0x0d:  enable = node->core.armv3.apsr_z || 
                             (node->core.armv3.apsr_n != 
                             node->core.armv3.apsr_v); break;   // le
        case 0x0e:  enable = 1; break;                          // always
        default:    enable = 0; break;                          // never
    }

    if (enable) {
        node->core.armv3.reg[15] = node->pc+8;
        switch ((iw>>25) & 7) {
            case 0x00:
            case 0x01:
            {
                if ((iw&0x0e000090)==0x00000090) {
                    // ....000. ........ ........ 1..1....
                    if ((iw&0x0f0000f0)==0x00000090) {
                        // multiply instruction extension space
                        // ....0000 ........ ........ 1001....
                        // cccc0000 ....nnnn ddddssss 1001mmmm
                        switch (iw&0x01f00000) {
                            case 0x00000000: // mul Rd=sbz (ARMv2)
                                REGn = REGm*REGs;
                                return ARMV3_LATENCY_MUL;
                            case 0x00100000: // muls Rd=sbz (ARMv2)
                                instr_set_apsr_nz(node, 1, REGn = REGm*REGs); 
                                return ARMV3_LATENCY_MUL;
                            case 0x00200000: // mla (ARMv2)
                                REGn = REGm*REGs + REGd;
                                return ARMV3_LATENCY_MLA;
                            case 0x00300000: // mlas (ARMv2)
                                instr_set_apsr_nz(node, 1, REGn = REGm*REGs + REGd); 
                                return ARMV3_LATENCY_MLA;
                            case 0x00400000: // umaal (ARMv6)
                                return ic_mull(node, iw, REGmU*REGsU + REGdU + REGnU);
                            case 0x00500000:
                            case 0x00600000:
                            case 0x00700000:
                                break; // undefined by ARM spec
                            case 0x00800000: // umull (ARMv3M)
                            case 0x00900000: // umulls (ARMv3M)
                                return ic_mull(node, iw, REGmU*REGsU);
                            case 0x00a00000: // umlal (ARMv3M)
                            case 0x00b00000: // umlals (ARMv3M)
                                return ic_mull(node, iw, REGmU*REGsU + (REGnU<<32) + REGdU);
                            case 0x00c00000: // smull (ARMv3M)
                            case 0x00d00000: // smulls (ARMv3M)
                                return ic_mull(node, iw, REGmS*REGsS);
                            case 0x00e00000: // smlal (ARMv3M)
                            case 0x00f00000: // smlals (ARMv3M)
                                return ic_mull(node, iw, REGmS*REGsS + (REGnU<<32) + REGdU);
                        }
                        break;
                    } else if ((iw&0x0f0000f0)==0x01000090) {
                        // load/store instruction extension space, part 2
                        //     ....0001 ........ ........ 1001....
                        break;
                    } else {
                        // load/store instruction extension space, part 1
                        // ....000. ........ ........ 1xx1....
                        // ....000P UIWxnnnn ddddssss 1xx1mmmm
                        addr_t ofs = (iw&0x00400000) // I
                            ? (iw&0x0000000f) | ((iw>>4)&0x000000f0)
                            : REGm; // Rs=sbz
                        addr_t base = REGn;
                        addr_t sum =  (iw&0x00800000) ? base+ofs : base-ofs; // U
                        addr_t addr = (iw&0x01000000) ? sum : base; // P

                        // Don't write register if offset addressing (P=1)
                        // without writeback (W=0). Otherwise write, although
                        // there are some "unpredictable" cases in the spec.
                        if ((iw&0x01200000)!=0x01000000) {
                            REGn = sum;
                        }

                        switch (iw&0x00100060) {
                            case 0x00000020: // strh (ARMv4)
                                return ic_store(node, iw, MA_16le, addr);
                            case 0x00000040: // ldrd (ARMv5TE)
                            {
                                if ((iw&0x1000)!=0) break; // Rd must be even
                                uint64_t u64;
                                instruction_class_t latency =
                                    2*memory_load_u64_direct(node, MA_u64le, addr, &u64);
                                REGd = u64 & 0xffffffff;
                                REGdODD = u64 >> 32;
                                return ARMV3_LATENCY_LOAD + latency;
                            }
                            case 0x00000060: // strd (ARMv5TE)
                                if ((iw&0x1000)!=0) break; // Rd must be even
                                return ARMV3_LATENCY_STORE +
                                    memory_store_direct(node, MA_u32le, addr, REGd) +
                                    memory_store_direct(node, MA_u32le, addr+4, REGdODD);
                            case 0x00100020: // ldrh (ARMv4)
                                return ic_load(node, iw, MA_u16le, addr);
                            case 0x00100040: // ldrsb (ARMv4)
                                return ic_load(node, iw, MA_8, addr);
                            case 0x00100060: // ldrsh (ARMv4)
                                return ic_load(node, iw, MA_16le, addr);
                        }
                    }
                    break;
                }
                if ((iw&0x0d900000)==0x01000000) {
                    // control and dsp instruction extension space
                    // ....00.1 0..0.... ........ ........
                    // original function: tst, teq, cmp or cmn without changing the flags
                    if ((iw&0x0ffffff0)==0x012fff10) {
                        // FIXME: change to thumb mode if last bit is set
                        node->nextpc = REGm & ~3;
//printf("BX R%lu\n", BRu(3,0));
                        return ARMV3_LATENCY_IJUMP; // bx (ARMv4T)
                    }
                    if ((iw&0x0fff0ff0)==0x016f0f10) { // clz
                        REGd = instr_clz32(REGm);
                        return ARMV3_LATENCY_ARITH;
                    }
                }


                bool logical_op = ((iw&0x00c00000)==0) || ((iw&0x01800000)==0x01800000);
                    // AND, EOR, TST, TEQ, ORR, MOV, BIC, MVN
                bool setcond = (iw&0x00100000)!=0;
                bool setcondshift = logical_op && setcond;
                uint32_t op2;
                if ((iw&0x02000000)==0) {
                    op2 = instr_shifted_reg(node, setcondshift, iw);
                } else {
                    uint32_t value = BRu(7, 0);
                    uint_fast16_t shift = 2*BRu(11, 8);
                    op2 = (value >> shift) | (value << (32-shift));
                }

                switch ((iw>>21) & 15) {
                    case 0x00: instr_set_apsr_nz(node, setcond, REGd = REGn & op2); break; // and
                    case 0x01: instr_set_apsr_nz(node, setcond, REGd = REGn ^ op2); break; // eor
                    case 0x02: REGd = instr_add_with_carry(node, setcond, REGn, ~op2, 1); break; // sub
                    case 0x03: REGd = instr_add_with_carry(node, setcond, ~REGn, op2, 1); break; // rsb
                    case 0x04: REGd = instr_add_with_carry(node, setcond, REGn, op2, 0); break; // add
                    case 0x05: REGd = instr_add_with_carry(node, setcond, REGn, op2, 
                                                            node->core.armv3.apsr_c); break; // adc
                    case 0x06: REGd = instr_add_with_carry(node, setcond, REGn, ~op2, 
                                                            node->core.armv3.apsr_c); break; // sbc
                    case 0x07: REGd = instr_add_with_carry(node, setcond, ~REGn, op2, 
                                                            node->core.armv3.apsr_c); break; // rsc
                    case 0x08: instr_set_apsr_nz(node, setcond, REGn & op2); break; // tst
                    case 0x09: instr_set_apsr_nz(node, setcond, REGn ^ op2); break; // teq
                    case 0x0a: instr_add_with_carry(node, setcond, REGn, ~op2, 1); break; // cmp
                    case 0x0b: instr_add_with_carry(node, setcond, REGn, op2, 0); break; // cmn
                    case 0x0c: instr_set_apsr_nz(node, setcond, REGd = REGn | op2); break; // orr
                    case 0x0d: instr_set_apsr_nz(node, setcond, REGd = op2); break; // mov
                    case 0x0e: instr_set_apsr_nz(node, setcond, REGd = REGn & (~op2)); break; // bic
                    case 0x0f: instr_set_apsr_nz(node, setcond, REGd = ~op2); break; // mvn
                }

                return (((iw&0x02000090)==0x00000010) ? ARMV3_LATENCY_SHIFT : ARMV3_LATENCY_ARITH) +
                       (((iw&0x0000f000)==0x0000f000) ? ARMV3_LATENCY_WRITEPC : 0);
                    // higher latency only for register shifts
            }
            case 0x02:
            case 0x03:
            {
                uint32_t ofs = ((iw&0x02000000)==0)
                    ? BRu(11, 0)
                    : instr_shifted_reg(node, false, iw);
                uint32_t addr;
                switch (iw&0x01a00000) {
                    case 0x00000000: addr = REGn;     REGn = REGn-ofs; break;
                    case 0x00200000: addr = REGn;     REGn = REGn-ofs; break; // ldrt/strt
                    case 0x00800000: addr = REGn;     REGn = REGn+ofs; break;
                    case 0x00a00000: addr = REGn;     REGn = REGn+ofs; break; // ldrt/strt
                    case 0x01000000: addr = REGn-ofs; break;
                    case 0x01200000: addr = REGn-ofs; REGn = REGn-ofs; break;
                    case 0x01800000: addr = REGn+ofs; break;
                    default:
                    case 0x01a00000: addr = REGn+ofs; REGn = REGn+ofs; break;
                }
                switch (iw&0x00500000) {
                    case 0x00000000: return ic_store(node, iw, MA_32le, addr);      // str
                    case 0x00100000: return ic_load (node, iw, MA_32le, addr);      // ldr
                    case 0x00400000: return ic_store(node, iw, MA_8, addr);         // strb
                    case 0x00500000: return ic_load (node, iw, MA_u8, addr);        // ldrb
                }
            }

            case 0x04:
            {
                // TODO: S bit behaviour
                uint32_t addr;
                uint_fast16_t len=0, i;

                for (i=0; i<16; i++) if (iw&(1<<i)) len++;

                uint32_t unchanged_basereg = REGn;
                switch (iw&0x01a00000) {
                    case 0x00000000: addr = REGn-4*len+4; break; // post decrement
                    case 0x00200000: addr = REGn-4*len+4; REGn = REGn-4*len; break;
                    case 0x00800000: addr = REGn; break; // post increment
                    case 0x00a00000: addr = REGn;         REGn = REGn+4*len; break;
                    case 0x01000000: addr = REGn-4*len; break; // pre decrement
                    case 0x01200000: addr = REGn-4*len;   REGn = REGn-4*len; break;
                    case 0x01800000: addr = REGn+4; break; // pre increment
                    default:
                    case 0x01a00000: addr = REGn+4;       REGn = REGn+4*len; break;
                }
                uint32_t first_addr = addr;

                uint_fast16_t latency=0; 
                for (i=0; i<15; i++) {
                    if (iw&(1<<i)) {
                        if ((iw&0x00100000)==0) {
                            latency += memory_store_direct(node, MA_32le, addr, node->core.armv3.reg[i]);
                        } else {
                            uint64_t u64;
                            latency += memory_load_u64_direct(node, MA_32le, addr, &u64);
                            node->core.armv3.reg[i] = u64;
                        }
                        addr = addr + 4;
                    }
                }
                if ((iw&0x00008000)!=0) {
                    if ((iw&0x00100000)==0) {
                        latency += memory_store_direct(node, MA_32le, addr, node->pc + 12);
                    } else {
                        uint64_t newpc;
                        latency += ARMV3_LATENCY_LOAD +
                            memory_load_u64_direct(node, MA_32le, addr, &newpc);
                        node->nextpc = newpc;
                    }
                }
                // Correction if STM with write base register in the list.
                // This solution is not absolutely correct, because the memory
                // location is written twice and in the wrong order. But this
                // should only under very special circumstances be a problem.
                if ((iw&0x00300000)==0x00200000) {// stm and write back (L=0,W=1)
                    uint_fast16_t basereg = BRu(19, 16);
                    if ((iw&((2<<basereg)-1)) == (1<<basereg)) {
                        // basereg bit is set and all lower bits are clear
                        memory_store_direct(node, MA_32le, first_addr, unchanged_basereg);
                    }
                }
                return ((iw&0x00100000)==0)
                    ? ARMV3_LATENCY_STORE + latency
                    : ARMV3_LATENCY_LOAD + latency;
            }


            case 0x05:
                node->nextpc = node->pc + 8 + (BRs(23, 0)*4);
                if ((iw&0x01000000)==0) {
                    return ARMV3_LATENCY_JUMP; // b
                } else {
//printf("BL %0lx\n", node->nextpc);
                    REG_LR = node->pc + 4;
                    return ARMV3_LATENCY_CALL; // bl
                }
            case 0x07:
                if ((iw&0x01000000)==0x01000000) { // svc
//printf("SYSCALL %lx at #%lu. r0=%x\n", iw&0x00ffffff, node->cycle, node->core.armv3.reg[0]);
                    switch (iw & 0x00ffffff) {
                        case 0x00: // end simulation
                            node->state = CS_STOPPED;
                            return IC_STOP;

                        case 0x01: // print character
                            core_printf(node->rank, "%c", node->core.armv3.reg[0]);
                            return 1;

                        case 0x50: // get number of cores
                            node->core.armv3.reg[0] = conf_max_rank;
                            return 1;

                        case 0x51: // get rank of this core
                            node->core.armv3.reg[0] = node->rank;
                            return 1;

                        case 0x52: // send flit
                            if (node->noc_send_flit(node,
                                node->core.armv3.reg[0],
                                node->core.armv3.reg[1]))
                                    return 1;
                            node->state = CS_SEND_BLOCKED;
                            return IC_BLOCKED;

                        case 0x53: // receive flit from specific core
                            {
                                flit_t flit;

                                if (node->noc_recv_flit(node,
                                    node->core.armv3.reg[0],
                                    &flit))
                                { 
                                    node->core.armv3.reg[0] = flit;
                                    return 1;
                                }
                                node->state = CS_RECV_BLOCKED;
                                return IC_BLOCKED;
                            }

                        case 0x54: // congestion: send buffer full?
                            node->core.armv3.reg[0] = 
                                node->noc_sender_ready(node) ? 0 : 1;
                            return 1;

                        case 0x55: // wait for a flit from any core
                            {
                                rank_t rank = node->noc_probe_any(node);
                                if (rank<MAX_RANK) {
                                    node->core.armv3.reg[0] = rank;
                                    return 1;
                                }
                                node->state = CS_RECV_BLOCKED;
                                return IC_BLOCKED;
                            }

                        case 0x56: // probe specific core
                            node->core.armv3.reg[0] = node->noc_probe_rank(node, 
                                node->core.armv3.reg[0]) ? 1 : 0;
                            return 1;

                        case 0x57: // probe any core
                            node->core.armv3.reg[0] = node->noc_probe_any(node);
                            return 1;

                        default:
                            user_printf("Unknown SVC 0x%h", iw & 0x00ffffff);
                    }
                    return 1;
                }
        }
    }
    else return 1; // disabled instruction

    node->instruction_word = iw;
    node->state = CS_UNKNOWN_INSTRUCTION;
    return IC_STOP;
}


// Simulate one cycle
instruction_class_t armv3_one_cycle(node_t *node)
{
    uint32_t iw;
    uint_fast32_t pc = node->pc & ~1;

    memory_fetch_32le(node, pc, &iw);
    node->nextpc = pc + 4;
    node->retired++;
    return armv3_execute_iw(node, iw);
}


// Init context
void armv3_init_context(node_t *node)
{
    uint_fast32_t i;

    memory_init(node, MT_PAGED_32BIT, 0x100000000LL);

    node->cycle = 0;
    node->retired = 0;
    node->state = CS_RUNNING;
    node->pc = 0xa0000000;

    node->core_type = CT_armv3;
    node->one_cycle = &armv3_one_cycle;

    node->core.armv3.apsr_n = 0;
    node->core.armv3.apsr_z = 0;
    node->core.armv3.apsr_c = 0;
    node->core.armv3.apsr_v = 0;

    node->core.armv3.ipsr = 0;
    node->core.armv3.control_npriv = 0;
    node->core.armv3.control_spsel = 0;
    node->core.armv3.primask_pm = 0;
    node->core.armv3.mode = ARMV3_MODE_THREAD;

    for (i = 0; i < 15; i++)
        node->core.armv3.reg[i] = 0;

    node->core.armv3.reg[13] = 0x00fffff8;
}


// Remove context from memory and free memory blocks
void armv3_finish_context(node_t *node)
{
    memory_finish(node);
}







// Disassemble an address and return the lengh of the instruction
int armv3_disasm(node_t *node, addr_t pc, char *dstr)
{
    uint32_t iw;
    memory_fetch_32le(node, pc, &iw);
//    return armv3_disasm_iw(dstr, pc, iw);
    sprintf(dstr, ".word\t0x%8"PRIx32, iw);
    return 4;
}


// Print a register dump
void armv3_print_context(node_t *node)
{
    char dstr[128];
    uint32_t iw;
    memory_fetch_32le(node, node->pc, &iw);
//    armv3_disasm_iw(dstr, node->pc, iw);
    sprintf(dstr, ".word\t0x%8"PRIx32, iw);

    user_printf("cycle %ld  %c%c%c%c pc=%08x\t%s\n"
                "  %08x %08x %08x %08x  %08x %08x %08x %08x\n"
                "  %08x %08x %08x %08x  %08x SP=%08x LR=%08x\n",
                node->cycle,
                node->core.armv3.apsr_n ? 'N' : '-',
                node->core.armv3.apsr_z ? 'Z' : '-',
                node->core.armv3.apsr_c ? 'C' : '-',
                node->core.armv3.apsr_v ? 'V' : '-',
                node->pc, dstr,
                node->core.armv3.reg[0],  node->core.armv3.reg[1],  node->core.armv3.reg[2],  node->core.armv3.reg[3],
                node->core.armv3.reg[4],  node->core.armv3.reg[5],  node->core.armv3.reg[6],  node->core.armv3.reg[7],
                node->core.armv3.reg[8],  node->core.armv3.reg[9],  node->core.armv3.reg[10], node->core.armv3.reg[11],
                node->core.armv3.reg[12], node->core.armv3.reg[13], node->core.armv3.reg[14]);
}
