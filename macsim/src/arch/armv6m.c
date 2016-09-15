/**
 * armv6m.c
 * ARM v6-M ISA
 *
 * MacSim project
 *
 *
 * Restrictions:
 *   - PUSH/POP/STM/LDM/SVC directly access memory, ignoring mapping of devices and
 *     network interface
 *
 */

#include "armv6m.h"
#include "memory.h"
#include <inttypes.h>
#include <string.h>


#define ARMV6M_MODE_HANDLER	1
#define ARMV6M_MODE_THREAD	2

// latency of instructions (in clock cycles)
#define ARMV6M_LATENCY_ARITH		1
#define ARMV6M_LATENCY_ADDR_CALC	1
#define ARMV6M_LATENCY_MUL		32
#define ARMV6M_LATENCY_BRANCH_NOTTAKEN	1
#define ARMV6M_LATENCY_BRANCH_TAKEN	3
#define ARMV6M_LATENCY_IJUMP		3
#define ARMV6M_LATENCY_ICALL		4
#define ARMV6M_LATENCY_JUMP		3
#define ARMV6M_LATENCY_CALL		3
#define ARMV6M_LATENCY_SYS		3

// Memory mapping for Network Interface Controller
#define ARMV6M_MEMMASK_NIC	0xfffff000
#define ARMV6M_MEMBASE_NIC	0x00800000




// private functions
uint32_t armv6m_raise_exception(node_t *node, uint32_t num, uint32_t addr);
uint32_t armv6m_leave_exception(node_t *node, uint32_t exc_return);





static inline instruction_class_t memory_load_u32_direct(
    node_t *node, unsigned access_type, addr_t addr, uint32_t *dest)
{
    uint64_t d;
    instruction_class_t r = memory_load_u64_direct(node, access_type, addr, &d);
    *dest = d;
    return r;
}



static inline instruction_class_t ic_load(node_t *node, unsigned access_type,
        addr_t addr, uint32_t *dest)
{
#ifdef ARMV6M_DEVICE_SUPPORT
    if (IS_DEVS_ADDR(node, addr)) {
        uint64_t d;
        instruction_class_t r = memory_load_u64_dev(node, access_type, addr, &d);
        *dest = d;
        return ARMV6M_LATENCY_ADDR_CALC + r;
    }
#endif
    return ARMV6M_LATENCY_ADDR_CALC + memory_load_u32_direct(node, access_type, addr, dest);
}

static inline instruction_class_t ic_store(node_t *node, unsigned access_type,
        addr_t addr, uint32_t data)
{
#ifdef ARMV6M_DEVICE_SUPPORT
    if (IS_DEVS_ADDR(node, addr))
        return ARMV6M_LATENCY_ADDR_CALC + memory_store_dev(node, access_type, addr, data);
#endif
    return ARMV6M_LATENCY_ADDR_CALC + memory_store_direct(node, access_type, addr, data);
}

static inline instruction_class_t ic_branch(node_t *node, bool cond, int8_t disp)
{
    if (cond)
    {
        node->nextpc = node->pc + 4 + 2*((int32_t)disp);
        return ARMV6M_LATENCY_BRANCH_TAKEN;
    }

    return ARMV6M_LATENCY_BRANCH_NOTTAKEN;
}

static inline void instr_set_apsr_nz(node_t *node, uint32_t result)
{
    node->core.armv6m.apsr_n = (result >> 31)&1;
    node->core.armv6m.apsr_z = (result & 0xffffffff) == 0 ? 1 : 0;
}

static inline uint_fast32_t instr_lsl(node_t *node, uint32_t value, uint32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value << shift;
        node->core.armv6m.apsr_c = (value >> (32-shift)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        result = 0;
        node->core.armv6m.apsr_c = (shift==32) ? (value & 1) : 0;
    }

    instr_set_apsr_nz(node, result);
    return result;
}

static inline uint_fast32_t instr_lsr(node_t *node, uint32_t value, uint32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value >> shift; // unsigned
        node->core.armv6m.apsr_c = (value >> (shift-1)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        result = 0;
        node->core.armv6m.apsr_c = (shift==32) ? ((value >> 31) & 1): 0;
    }

    instr_set_apsr_nz(node, result);
    return result;
}

static inline uint_fast32_t instr_asr(node_t *node, int32_t value, int32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else if (shift < 32)
    {
        result = value >> shift; // signed
        node->core.armv6m.apsr_c = (value >> (shift-1)) & 1;
    }
    else
    {
        // shift amount >= bit width is undefined in C, handle seperately
        node->core.armv6m.apsr_c = (value >> 31) & 1;
        result = 0 - node->core.armv6m.apsr_c;
    }

    instr_set_apsr_nz(node, result);
    return result;
}

static inline uint_fast32_t instr_ror(node_t *node, uint32_t value, uint_fast32_t shift)
{
    uint32_t result;

    if (shift==0)
        result = value;
    else
    {
        result = (value >> shift) | (value << (32-shift));
        node->core.armv6m.apsr_c = (result >> 31) & 1;
    }

    instr_set_apsr_nz(node, result);
    return result;
}



static inline uint_fast32_t instr_add_with_carry(node_t *node, uint32_t a, uint32_t b, uint_fast64_t carry)
// side effect: set PSR flags
{
    uint_fast64_t r = (uint_fast64_t)a + (uint_fast64_t)b + carry;
    instr_set_apsr_nz(node, r);
    node->core.armv6m.apsr_c = (r >> 32)&1;
//    int64_min_t s = (int64_min_t)a + (int64_min_t)b + carry;
//    node->core.armv6m.apsr_v =  (s != (((int64_min_t)(r<<32)) >> 32)) ? 1 : 0;
    node->core.armv6m.apsr_v = ((a >> 31)^(r >> 31))&((b >> 31)^(r >> 31)) & 1;
    // only set if pos+pos=neg or neg+neg=pos
    return r;
}

static inline uint_fast32_t build_psr(node_t *node)
{
    return (node->core.armv6m.apsr_n<<31)
           | (node->core.armv6m.apsr_z<<30)
           | (node->core.armv6m.apsr_c<<29)
           | (node->core.armv6m.apsr_v<<28)
//	| 0x00000000 // thumb state bit reads as 0
           | (node->core.armv6m.ipsr & 0x3f);
}

static inline void restore_apsr(node_t *node, uint32_t psr) {
  node->core.armv6m.apsr_n = (psr >> 31) & 1;
  node->core.armv6m.apsr_z = (psr >> 30) & 1;
  node->core.armv6m.apsr_c = (psr >> 29) & 1;
  node->core.armv6m.apsr_v = (psr >> 28) & 1;
}


#define BRu(h,l)	(((iw)>>(l))&((1<<((h)-(l)+1))-1))
#define BRs(h,l)	((((int32_t)(iw))<<(31-(h)))>>(31-(h)+(l)))

#define REG_Au		node->core.armv6m.reg[BRu(2, 0)]
#define REG_Bu		node->core.armv6m.reg[BRu(5, 3)]
#define REG_Cu		node->core.armv6m.reg[BRu(8, 6)]
#define REG_Du		node->core.armv6m.reg[BRu(10, 8)]
#define REG_As		((int32_t)node->core.armv6m.reg[BRu(2, 0)])
#define REG_Bs		((int32_t)node->core.armv6m.reg[BRu(5, 3)])

#define REG_LR		node->core.armv6m.reg[14]
#define REG_SP		node->core.armv6m.reg[13]

#define IMM_3		BRu(8, 6)
#define IMM_5		BRu(10, 6)
#define IMM_7		BRu(6, 0)
#define IMM_8		BRu(7, 0)



// ---------------------------------------------------------------------
// Main switch for decoding and execution
// ---------------------------------------------------------------------


instruction_class_t armv6m_execute_iw(node_t *node, uint_fast32_t iw)
{
    switch ((iw>>11) & 0x1f)
    {
        case 0x00: // lsls
            REG_Au = instr_lsl(node, REG_Bu, IMM_5);
            return ARMV6M_LATENCY_ARITH;

        case 0x01: // lsrs
            {
                unsigned count = IMM_5;

                if (count==0) count = 32;

                REG_Au = instr_lsr(node, REG_Bu, count);
                return ARMV6M_LATENCY_ARITH;
            }

        case 0x02: // asrs
            {
                unsigned count = IMM_5;

                if (count==0) count = 32;

                REG_Au = instr_asr(node, REG_Bu, count);
                return ARMV6M_LATENCY_ARITH;
            }

        case 0x03:
            switch (BRu(10, 9))
            {
                case 0x00: // adds (reg)
                    REG_Au = instr_add_with_carry(node, REG_Bu, REG_Cu, 0);
                    return ARMV6M_LATENCY_ARITH;

                case 0x01: // subs (reg)
                    REG_Au = instr_add_with_carry(node, REG_Bu, ~(REG_Cu), 1);
                    return ARMV6M_LATENCY_ARITH;

                case 0x02: // adds (imm3)
                    REG_Au = instr_add_with_carry(node, REG_Bu, IMM_3, 0);
                    return ARMV6M_LATENCY_ARITH;

                case 0x03: // subs (imm3)
                    REG_Au = instr_add_with_carry(node, REG_Bu, ~(IMM_3), 1);
                    return ARMV6M_LATENCY_ARITH;
            }

        case 0x04: // movs
            REG_Du = IMM_8;
            node->core.armv6m.apsr_n = 0;
            node->core.armv6m.apsr_z = (IMM_8 == 0) ? 0 : 1;
            return ARMV6M_LATENCY_ARITH;

        case 0x05: // cmp (imm8)
            instr_add_with_carry(node, REG_Du, ~(IMM_8), 1);
            return ARMV6M_LATENCY_ARITH;

        case 0x06: // adds (imm8)
            REG_Du = instr_add_with_carry(node, REG_Du, IMM_8, 0);
            return ARMV6M_LATENCY_ARITH;

        case 0x07: // subs (imm8)
            REG_Du = instr_add_with_carry(node, REG_Du, ~(IMM_8), 1);
            return ARMV6M_LATENCY_ARITH;

        case 0x08:
            switch (BRu(10, 6))
            {
                case 0x00: // ands
                    instr_set_apsr_nz(node, REG_Au = REG_Au & REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x01: // eors
                    instr_set_apsr_nz(node, REG_Au = REG_Au ^ REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x02: // lsls
                    REG_Au = instr_lsl(node, REG_Au, REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x03: // lsrs
                    REG_Au = instr_lsr(node, REG_Au, REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x04: // asrs
                    REG_Au = instr_asr(node, REG_Au, REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x05: // adcs
                    REG_Au = instr_add_with_carry(node, REG_Au, REG_Bu, node->core.armv6m.apsr_c);
                    return ARMV6M_LATENCY_ARITH;

                case 0x06: // sbcs
                    REG_Au = instr_add_with_carry(node, REG_Au, ~REG_Bu, node->core.armv6m.apsr_c);
                    return ARMV6M_LATENCY_ARITH;

                case 0x07: // ror
                    REG_Au = instr_ror(node, REG_Au, REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x08: // tst
                    instr_set_apsr_nz(node, REG_Au & REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x09: // rsbs
                    REG_Au = instr_add_with_carry(node, ~REG_Bu, 0, 1);
                    return ARMV6M_LATENCY_ARITH;

                case 0x0a: // cmp
                    instr_add_with_carry(node, REG_Au, ~REG_Bu, 1);
                    return ARMV6M_LATENCY_ARITH;

                case 0x0b: // cmn
                    instr_add_with_carry(node, REG_Au, REG_Bu, 0);
                    return ARMV6M_LATENCY_ARITH;

                case 0x0c: // orrds
                    instr_set_apsr_nz(node, REG_Au = REG_Au | REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x0d: // muls
                    instr_set_apsr_nz(node, REG_Au = REG_Au * REG_Bu);
                    return ARMV6M_LATENCY_MUL;

                case 0x0e: // bics
                    instr_set_apsr_nz(node, REG_Au = REG_Au & ~REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x0f: // mvns
                    instr_set_apsr_nz(node, REG_Au = ~REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                default:
                    {
                        // the upper 8 register can only be accessed within this opcode
                        // range
                        uint_fast16_t reg_a = BRu(2, 0) | ((iw>>4)&8);
                        uint_fast16_t reg_b = BRu(6, 3);
                        uint32_t value_a = (reg_a==15)
                                           ? (node->pc+4)
                                           : node->core.armv6m.reg[reg_a];
                        uint32_t value_b = (reg_b==15)
                                           ? (node->pc+4)
                                           : node->core.armv6m.reg[reg_b];

                        switch (BRu(9, 7))
                        {
                            case 0x00:
                            case 0x01: // add (don't set flags)
                                if (reg_a==15)
                                {
                                    node->nextpc = (value_a + value_b) & ~1;
                                    return ARMV6M_LATENCY_IJUMP;
                                }

                                node->core.armv6m.reg[reg_a] = value_a + value_b;
                                return ARMV6M_LATENCY_ARITH;

                            case 0x02:
                            case 0x03: // cmp (r16)
                                instr_add_with_carry(node, value_a, ~value_b, 1);
                                return ARMV6M_LATENCY_ARITH;

                            case 0x04:
                            case 0x05: // mov (don't set flags)
                                if (reg_a==15)
                                {
                                    node->nextpc = value_b & ~1;
                                    return ARMV6M_LATENCY_IJUMP;
                                }

                                node->core.armv6m.reg[reg_a] = value_b;
                                return ARMV6M_LATENCY_ARITH;

                            case 0x06: // bx
                                if (reg_b==15)
                                    // special case: 'bx pc' to switch to thumb mode
                                    node->nextpc = node->pc + 4;
                                else {
				  if (__builtin_expect((value_b & 0xf0000000) == 0xf0000000, 0)) {
				    return armv6m_leave_exception(node, value_b);
				  }
				  else {
                                    node->nextpc = value_b & ~1;
				  }
				}

                                return ARMV6M_LATENCY_IJUMP;

                            case 0x07: // blx
                                REG_LR = node->pc + 3; // next instruction and set thumb bit
                                node->nextpc = value_b & ~1;
                                return ARMV6M_LATENCY_ICALL;
                        }
                    }
            }

        case 0x09: // ldr (literal)
            return ic_load(node, MA_u32le, (node->pc & ~3) + 4 + 4*IMM_8, &REG_Du);

        case 0x0a:
            switch (BRu(10, 9))
            {
                case 0x00:
                    return ic_store(node, MA_32le, REG_Bu+REG_Cu, REG_Au);	// str (reg)

                case 0x01:
                    return ic_store(node, MA_16le, REG_Bu+REG_Cu, REG_Au);	// strh (reg)

                case 0x02:
                    return ic_store(node, MA_8,    REG_Bu+REG_Cu, REG_Au);	// strb (reg)

                case 0x03:
                    return ic_load(node,  MA_8,    REG_Bu+REG_Cu, &REG_Au);	// ldrsb (reg)
            }

        case 0x0b:
            switch (BRu(10, 9))
            {
                case 0x00:
                    return ic_load(node, MA_32le,  REG_Bu+REG_Cu, &REG_Au);	// ldr (reg)

                case 0x01:
                    return ic_load(node, MA_u16le, REG_Bu+REG_Cu, &REG_Au);	// ldrh (reg)

                case 0x02:
                    return ic_load(node, MA_u8,    REG_Bu+REG_Cu, &REG_Au);	// ldrb (reg)

                case 0x03:
                    return ic_load(node, MA_16le,  REG_Bu+REG_Cu, &REG_Au);	// ldrsh (reg)
            }

        case 0x0c:
            return ic_store(node, MA_32le,  REG_Bu + 4*IMM_5, REG_Au);	// str (imm5)

        case 0x0d:
            return ic_load (node, MA_32le,  REG_Bu + 4*IMM_5, &REG_Au);	// ldr

        case 0x0e:
            return ic_store(node, MA_8,     REG_Bu + IMM_5,   REG_Au);	// strb

        case 0x0f:
            return ic_load (node, MA_u8,    REG_Bu + IMM_5,   &REG_Au);	// ldrb

        case 0x10:
            return ic_store(node, MA_16le,  REG_Bu + 2*IMM_5, REG_Au);	// strh

        case 0x11:
            return ic_load (node, MA_u16le, REG_Bu + 2*IMM_5, &REG_Au);	// ldrh

        case 0x12:
            return ic_store(node, MA_32le,  REG_SP + 4*IMM_8, REG_Du);	// str (sp+imm8)

        case 0x13:
            return ic_load (node, MA_32le,  REG_SP + 4*IMM_8, &REG_Du);	// ldr (sp+imm8)

        case 0x14: // adr
            REG_Du = (node->pc & ~3) + 4 + 4*IMM_8;
            return ARMV6M_LATENCY_ARITH;

        case 0x15: // add reg, sp, imm8
            REG_Du = REG_SP + 4*IMM_8;
            return ARMV6M_LATENCY_ARITH;

        case 0x16:
            switch (BRu(10, 6))
            {
                case 0x00:
                case 0x01: // add sp, imm7
                    REG_SP += 4*IMM_7;
                    return ARMV6M_LATENCY_ARITH;

                case 0x02:
                case 0x03: // sub sp, imm7
                    REG_SP -= 4*IMM_7;
                    return ARMV6M_LATENCY_ARITH;

                    // ...
                case 0x08: // sxth
                    REG_Au = (int32_t)(int16_t)REG_Bu;
                    return ARMV6M_LATENCY_ARITH;

                case 0x09: // sxtb
                    REG_Au = (int32_t)(int8_t)REG_Bu;
                    return ARMV6M_LATENCY_ARITH;

                case 0x0a: // uxth
                    REG_Au = REG_Bu & 0xffff;
                    return ARMV6M_LATENCY_ARITH;

                case 0x0b: // uxtb
                    REG_Au = REG_Bu & 0xff;
                    return ARMV6M_LATENCY_ARITH;

                    // ...
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17: // push
                    {
                        // TODO: Also mapped memory accesses. Problem: can block!
                        int i;
                        addr_t sp = REG_SP;
                        cycle_t latency = 1;

                        if (iw&0x00100)
                        {
                            sp -= 4;
                            latency += memory_store_direct(node, MA_32le, sp, REG_LR);
                        }

                        for (i=7; i>=0; i--)
                        {
                            if ((iw&(1<<i))!=0)
                            {
                                sp -= 4;
                                latency += memory_store_direct(node, MA_32le, sp, node->core.armv6m.reg[i]);
                            }
                        }

                        REG_SP = sp;
                        return latency;
                    }

                    // ...
                case 0x19:
                    if (iw==0xb662) // cpsie i
                    {
                        if (node->core.armv6m.mode==ARMV6M_MODE_HANDLER
                                || node->core.armv6m.control_npriv==0)
                            node->core.armv6m.primask_pm = 0;

                        return ARMV6M_LATENCY_ARITH;
                    }
                    else if (iw==0xb673) // cpsid i
                    {
                        if (node->core.armv6m.mode==ARMV6M_MODE_HANDLER
                                || node->core.armv6m.control_npriv==0)
                            node->core.armv6m.primask_pm = 1;

                        return ARMV6M_LATENCY_ARITH;
                    }

                    break;
                    // ...
            }

            break;

        case 0x17:
            switch (BRu(10, 6))
            {
                    // ...
                case 0x08: // rev
                    REG_Au = __builtin_bswap32(REG_Bu);
                    return ARMV6M_LATENCY_ARITH;

                case 0x09: // rev16
                    REG_Au = ((REG_Bu<<8)&0xff00ff00) | ((REG_Bu>>8)&0x00ff00ff);
                    return ARMV6M_LATENCY_ARITH;

                    // ...
                case 0x0b: // revsh
                    REG_Au = (int32_t)(int16_t)( ((REG_Bu&0xff)<<8) | ((REG_Bu>>8)&0xff) );
                    return ARMV6M_LATENCY_ARITH;

                    // ...
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17: // pop
                    {
                        // TODO: Also mapped memory accesses. Problem: can block!
                        int i;
                        addr_t sp = REG_SP;
                        cycle_t latency = 1;
			bool eret = false;
			uint32_t nextpc;

                        for (i=0; i<8; i++)
                            if ((iw&(1<<i))!=0)
                            {
                                latency += memory_load_u32_direct(node, MA_32le, sp, &node->core.armv6m.reg[i]);
                                sp += 4;
                            }

                        if ((iw&0x00100)!=0)
                        {
                            latency += memory_load_u32_direct(node, MA_32le, sp, &nextpc);
                            sp += 4;
			    if (__builtin_expect((nextpc & 0xf0000000) == 0xf0000000, 0)) {
			      eret = true;
			    }
			    else {
			      node->nextpc = nextpc & ~1;
			    }
                        }

                        REG_SP = sp;
			if (eret) {
			  latency += armv6m_leave_exception(node, nextpc & ~1);
			}
                        return latency;
                    }

                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b: // bkpt: end simulation
                    node->state = CS_STOPPED;
                    return IC_STOP;
                    /*
                    	case 0x1c:
                    	case 0x1d:
                    	case 0x1e:
                    	case 0x1f: // hint: nop, yield, wfe, wfi, sev
                    */
            }

        case 0x18: // stm
            {
                // TODO: Also mapped memory accesses. Problem: can block!
                // If REG_Du is in the list, its original value i stored
                int i;
                addr_t a = REG_Du;
                cycle_t latency = 1;

                for (i=0; i<8; i++)
                    if ((iw&(1<<i))!=0)
                    {
                        latency += memory_store_direct(node, MA_32le, a, node->core.armv6m.reg[i]);
                        a += 4;
                    }

                REG_Du = a;
                return latency;
            }

        case 0x19: // ldm
            {
                int i;
                addr_t a = REG_Du;
                cycle_t latency = 1;

                for (i=0; i<8; i++)
                    if ((iw&(1<<i))!=0)
                    {
                        latency += memory_load_u32_direct(node, MA_32le, a, &node->core.armv6m.reg[i]);
                        a += 4;
                    }

                if ((iw&(1<<BRu(10, 8)))==0) REG_Du = a;

                // only write end address back if REG_Du is not in the list
                return latency;
            }

        case 0x1a:
            switch (BRu(10, 8))
            {
                case 0x00:
                    return ic_branch(node, node->core.armv6m.apsr_z, iw); // beq

                case 0x01:
                    return ic_branch(node, !node->core.armv6m.apsr_z, iw); // bne

                case 0x02:
                    return ic_branch(node, node->core.armv6m.apsr_c, iw); // bcs

                case 0x03:
                    return ic_branch(node, !node->core.armv6m.apsr_c, iw); // bcc

                case 0x04:
                    return ic_branch(node, node->core.armv6m.apsr_n, iw); // bmi

                case 0x05:
                    return ic_branch(node, !node->core.armv6m.apsr_n, iw); // bpl

                case 0x06:
                    return ic_branch(node, node->core.armv6m.apsr_v, iw); // bvs

                case 0x07:
                    return ic_branch(node, !node->core.armv6m.apsr_v, iw); // bvc
            }

        case 0x1b:
            switch (BRu(10, 8))
            {
                case 0x00:
                    return ic_branch(node, node->core.armv6m.apsr_c && !node->core.armv6m.apsr_z, iw); // bhi

                case 0x01:
                    return ic_branch(node, !node->core.armv6m.apsr_c || node->core.armv6m.apsr_z, iw); // bls

                case 0x02:
                    return ic_branch(node, node->core.armv6m.apsr_n == node->core.armv6m.apsr_v, iw); // bge

                case 0x03:
                    return ic_branch(node, node->core.armv6m.apsr_n != node->core.armv6m.apsr_v, iw); // blt

                case 0x04:
                    return ic_branch(node,
                                     !node->core.armv6m.apsr_z && (node->core.armv6m.apsr_n == node->core.armv6m.apsr_v), iw); // bgt

                case 0x05:
                    return ic_branch(node,
                                     node->core.armv6m.apsr_z || (node->core.armv6m.apsr_n != node->core.armv6m.apsr_v), iw); // ble

                case 0x06: // bal
                    node->nextpc = node->pc + 4 + 2*BRs(7, 0);
                    return ARMV6M_LATENCY_JUMP;

                case 0x07: // svc
                    switch (IMM_8)
                    {
                        case 0x01: // print character
                            core_printf(node->rank, "%c", node->core.armv6m.reg[0]);
                            return 1;

                        case ARMV6M_SVC_SYSCALL: // perform "real" system call
                            return armv6m_raise_exception(node, 
                                ARMV6M_EXC_SUPERVISOR_CALL, node->pc);

                        case 0x50: // get number of cores
                            node->core.armv6m.reg[0] = conf_max_rank;
                            return 1;

                        case 0x51: // get rank of this core
                            node->core.armv6m.reg[0] = node->rank;
                            return 1;

                        case 0x52: // send flit
                            if (node->noc_send_flit(node,
                                node->core.armv6m.reg[0],
                                node->core.armv6m.reg[1]))
                                    return 1;
                            node->state = CS_SEND_BLOCKED;
                            return IC_BLOCKED;

                        case 0x53: // receive flit from specific core
                            {
                                flit_t flit;

                                if (node->noc_recv_flit(node,
                                    node->core.armv6m.reg[0],
                                    &flit))
                                { 
                                    node->core.armv6m.reg[0] = flit;
                                    return 1;
                                }
                                node->state = CS_RECV_BLOCKED;
                                return IC_BLOCKED;
                            }

                        case 0x54: // congestion: send buffer full?
                            node->core.armv6m.reg[0] = 
                                node->noc_sender_ready(node) ? 0 : 1;
                            return 1;

                        case 0x55: // wait for a flit from any core
                            {
                                rank_t rank = node->noc_probe_any(node);
                                if (rank<MAX_RANK) {
                                    node->core.armv6m.reg[0] = rank;
                                    return 1;
                                }
                                node->state = CS_RECV_BLOCKED;
                                return IC_BLOCKED;
                            }

                        case 0x56: // probe specific core
                            node->core.armv6m.reg[0] = node->noc_probe_rank(node, 
                                node->core.armv6m.reg[0]) ? 1 : 0;
                            return 1;

                        case 0x57: // probe any core
                            node->core.armv6m.reg[0] = node->noc_probe_any(node);
                            return 1;

                        default:
                            {
                                // TODO: Also mapped memory accesses. Problem: can block!
                                addr_t sp = REG_SP - 32;
                                cycle_t latency;
                                uint32_t nextpc;

                                latency = memory_store_direct(node, MA_32le, sp+0, node->core.armv6m.reg[0]);
                                latency += memory_store_direct(node, MA_32le, sp+4, node->core.armv6m.reg[1]);
                                latency += memory_store_direct(node, MA_32le, sp+8, node->core.armv6m.reg[2]);
                                latency += memory_store_direct(node, MA_32le, sp+12, node->core.armv6m.reg[3]);
                                latency += memory_store_direct(node, MA_32le, sp+16, node->core.armv6m.reg[12]);
                                latency += memory_store_direct(node, MA_32le, sp+20, node->core.armv6m.reg[14]); // LR
                                latency += memory_store_direct(node, MA_32le, sp+24, node->pc + 4);
                                latency += memory_store_direct(node, MA_32le, sp+28, build_psr(node));// PSR
                                latency += memory_load_u32_direct(node, MA_32le, 0x0000002c, &nextpc);
                                node->nextpc = nextpc & ~1;
                                REG_SP = (uintptr_t)sp;
                                REG_LR = 0xfffffffd;
                                return latency;
                            }
                    }
            }

        case 0x1c: // b (imm11)
            node->nextpc = node->pc + 4 + 2*BRs(10, 0);
            return ARMV6M_LATENCY_JUMP;

            // ...
        case 0x1e:
            {
                uint16_t iw2;
                memory_fetch_16le(node, node->pc+2, &iw2);

                if ((iw2&0xd000)==0xd000) // bl
                {
                    int32_t s = (iw>>10)&1;
                    int32_t disp = (s << 24) // S
                                   | ((~((iw2>>13) ^ s)&1) << 23) // not(J1 xor S)
                                   | ((~((iw2>>11) ^ s)&1) << 22) // not(J2 xor S)
                                   | (BRu(9, 0) << 12)
                                   | ((iw2 & 0x7ff) << 1);
                    disp = (disp << 7) >> 7; // sign extension
                    REG_LR = node->pc + 5; // next instruction and thumb bit set
                    node->nextpc = node->pc + 4 + disp;
                    return ARMV6M_LATENCY_CALL;
                }

                if (((iw&0xffff)==0xf3ef) && ((iw2&0xf000)==0x8000)) // mrs
                {
                    int_fast32_t r = 0;
		    node->nextpc += 2; // 32-bit instruction!

                    switch (iw2 & 0xff)
                    {
                        case 0x00: // APSR
                        case 0x02: // EAPSR
                            r = build_psr(node) & ~0x3f;
                            break;

                        case 0x01: // IAPSR
                        case 0x03: // XPSR
                            r = build_psr(node);
                            break;

                        case 0x06: // IPSR
                            r = build_psr(node) & 0x3f;
                            break;

                        case 0x08:
			    if (node->core.armv6m.control_spsel
				&& node->core.armv6m.mode == ARMV6M_MODE_THREAD) {
			      r = node->core.armv6m.sp_main;
			    }
			    else {
			      // currently running on sp_main
			      r = REG_SP;
			    }
			    break;

                        case 0x09:
			    if (node->core.armv6m.control_spsel
				&& node->core.armv6m.mode == ARMV6M_MODE_THREAD) {
			      // currently running on sp_process
			      r = REG_SP;
			    }
			    else {
			      r = node->core.armv6m.sp_process;
			    }
                            break;

                        case 0x10:
                            r = node->core.armv6m.primask_pm;
                            break;

                        case 0x14:
                            r = (node->core.armv6m.control_spsel<<1)
                                | node->core.armv6m.control_npriv;
                            break;
                    }

                    node->core.armv6m.reg[((iw2>>8)&0x0f)] = r;
                    return ARMV6M_LATENCY_SYS;
                }

                if (((iw&0xfff0)==0xf380) && ((iw2&0xff00)==0x8800)) // msr
                {
                    int_fast32_t r = REG_Au;
		    node->nextpc += 2; // 32-bit instruction!

                    switch (iw2 & 0xff)
                    {
                        case 0x00: // APSR
                        case 0x01: // IAPSR
                        case 0x02: // EAPSR
                        case 0x03: // XPSR
                            node->core.armv6m.apsr_n = (r>>31) & 1;
                            node->core.armv6m.apsr_z = (r>>30) & 1;
                            node->core.armv6m.apsr_c = (r>>29) & 1;
                            node->core.armv6m.apsr_v = (r>>28) & 1;
                            return ARMV6M_LATENCY_SYS;

		        case 0x08: // SP_MAIN
			  if (node->core.armv6m.control_npriv==0) {
			    if (node->core.armv6m.control_spsel
				&& node->core.armv6m.mode == ARMV6M_MODE_THREAD) {
			      node->core.armv6m.sp_main = r & ~3;
			      //user_printf("[%d]msr: sp_main = %08x\n", node->rank, (r&~3));
			    }
			    else {
			      REG_SP = r & ~3;
			      //user_printf("[%d]msr: sp_main via SP = %08x\n", node->rank, (r&~3));
			    }
			  }
			  return ARMV6M_LATENCY_SYS;

		        case 0x09: // SP_PROCESS
			  if (node->core.armv6m.control_npriv==0) {
			    if (node->core.armv6m.control_spsel
				&& node->core.armv6m.mode == ARMV6M_MODE_THREAD) {
			      REG_SP = r & ~3;
			      //user_printf("[%d]msr: sp_process via SP =%08x\n", node->rank, (r&~3));
			    }
			    else {
			      //user_printf("[%d]msr: sp_process = %08x\n", node->rank, (r&~3));
			      node->core.armv6m.sp_process = r & ~3;
			    }
			  }
                            return ARMV6M_LATENCY_SYS;

		        case 0x10: // PRIMASK.PM
                            if (node->core.armv6m.control_npriv==0)
                                node->core.armv6m.primask_pm = r & 1;

                            return ARMV6M_LATENCY_SYS;

                        case 0x14:
                            if (node->core.armv6m.mode==ARMV6M_MODE_THREAD
                                    && node->core.armv6m.control_npriv==0)
                            {
			        bool osps = node->core.armv6m.control_spsel;
                                node->core.armv6m.control_spsel = (r >> 1) & 1;
				if (osps != node->core.armv6m.control_spsel) {
				  // switch stacks
				  if (node->core.armv6m.control_spsel) {
				    // to sp_process
				    node->core.armv6m.sp_main = REG_SP;
				    REG_SP = node->core.armv6m.sp_process;
				    //user_printf("[%d]msr: switched SP from %08x to sp_process = %08x\n", node->rank, node->core.armv6m.sp_main, REG_SP);
				  }
				  else {
				    // to sp_main
				    node->core.armv6m.sp_process = REG_SP;
				    REG_SP = node->core.armv6m.sp_main;
				    //user_printf("[%d]msr: switched SP from %08x to sp_main = %08x\n", node->rank, node->core.armv6m.sp_process, REG_SP);
				  }
				}
                                node->core.armv6m.control_npriv = r & 1;
                            }

                            return ARMV6M_LATENCY_SYS;
                    }
                }

                if (iw==0xf3bf)
                {
		    node->nextpc += 2; // 32-bit instruction!
                    switch (iw2 & 0xfff0)
                    {
                        case 0x8f40: // dsb
                        case 0x8f50: // dmb
                        case 0x8f60: // isb
                            return ARMV6M_LATENCY_SYS;
                    }
                }
            }

            // ...
    }

    node->instruction_word = iw;
    node->state = CS_UNKNOWN_INSTRUCTION;
    return IC_STOP;
}


// Simulate one cycle
instruction_class_t armv6m_one_cycle(node_t *node)
{
    uint16_t iw;
    uint_fast32_t pc = node->pc & ~1;
#ifdef ARMV6M_DEVICE_SUPPORT
    irqmap_t irq = node->irqmap;
    if (__builtin_expect( (irq != 0
               && node->core.armv6m.primask_pm == 0
               && node->core.armv6m.mode == ARMV6M_MODE_THREAD),
              0) ) {
        //user_printf("IRQ available: %x\n", irq);
        return armv6m_raise_exception(node, ARMV6M_EXC_EXTERNAL_IRQ, pc);
    }
#endif
    memory_fetch_16le(node, pc, &iw);
    node->nextpc = pc + 2;
    node->retired++;
    return armv6m_execute_iw(node, iw);
}


// Init context
void armv6m_init_context(node_t *node)
{
    uint_fast32_t i;

    memory_init(node, MT_PAGED_32BIT, 0x100000000LL);

    node->cycle = 0;
    node->retired = 0;
    node->state = CS_RUNNING;
    node->pc = 0xa0000000;

    node->core_type = CT_armv6m;
    node->one_cycle = &armv6m_one_cycle;

    node->core.armv6m.apsr_n = 0;
    node->core.armv6m.apsr_z = 0;
    node->core.armv6m.apsr_c = 0;
    node->core.armv6m.apsr_v = 0;

    node->core.armv6m.ipsr = 0;
    node->core.armv6m.control_npriv = 0;
    node->core.armv6m.control_spsel = 0;
    node->core.armv6m.primask_pm = 0;
    node->core.armv6m.mode = ARMV6M_MODE_THREAD;

    for (i = 0; i < 15; i++)
        node->core.armv6m.reg[i] = 0;

    node->core.armv6m.reg[13] = 0x00fffff8;

#ifdef ARMV6M_DEVICE_SUPPORT
    node->device_segment.base = ARMV6M_DEV_BASE;
    node->device_segment.len  = ARMV6M_DEV_LENGTH;
    node->device_segment.mask = ARMV6M_DEV_MASK;
#endif
}


// Remove context from memory and free memory blocks
void armv6m_finish_context(node_t *node)
{
    memory_finish(node);
}










static char *cat_reg(char *d, unsigned reg)
{
    switch (reg)
    {
        case 10:
        case 11:
        case 12:
            d[0]='r';
            d[1]='1';
            d[2]=reg-10+'0';
            return d+3;

        case 13:
            d[0]='s';
            d[1]='p';
            return d+2;

        case 14:
            d[0]='l';
            d[1]='r';
            return d+2;

        case 15:
            d[0]='p';
            d[1]='c';
            return d+2;

        default:
            d[0]='r';
            d[1]=reg+'0';
            return d+2;
    }
}


static char *cat_uint(char *d, uint_fast32_t imm)
{
    return d + sprintf(d, "#%"PRIuFAST32, imm);
}


static char *cat_addr(char *d, uint32_t a)
{
    return d + sprintf(d, "%"PRIx32, a);
}


static void disasm(char *d, const char *f, addr_t pc, uint_fast32_t iw)
{
    char c;

    do
    {
        c = *f++;

        if (c=='%')
        {
            c = *f++;

            switch (c)
            {
                case 'a':
                    d = cat_reg(d, BRu(2, 0));
                    break;

                case 'b':
                    d = cat_reg(d, BRu(5, 3));
                    break;

                case 'c':
                    d = cat_reg(d, BRu(8, 6));
                    break;

                case 'd':
                    d = cat_reg(d, BRu(10, 8));
                    break;

                case 'A':
                    d = cat_reg(d, BRu(2, 0)| ((iw>>4)&8));
                    break;

                case 'B':
                    d = cat_reg(d, BRu(6, 3));
                    break;

                case 'D':
                    d = cat_reg(d, BRu(27, 24));
                    break; // for mrs and msr

                case 'E':
                    d = cat_reg(d, BRu(3, 0));
                    break; // for mrs and msr

                case '3':
                    d = cat_uint(d, BRu(8, 6));
                    break;

                case '5':
                    d = cat_uint(d, BRu(10, 6));
                    break;

                case '6':
                    {
                        // for asrs and lsrs
                        unsigned u = BRu(10, 6);

                        if (u==0) u = 32;

                        d = cat_uint(d, u);
                        break;
                    }

                case '7':
                    d = cat_uint(d, BRu(6, 0));
                    break;

                case '8':
                    d = cat_uint(d, BRu(7, 0));
                    break;

                case 'W':
                    d = cat_uint(d, 2*BRu(10, 6));
                    break;

                case 'w':
                    d = cat_uint(d, 4*BRu(10, 6));
                    break;

                case 'y':
                    d = cat_uint(d, 4*BRu(6, 0));
                    break;

                case 'z':
                    d = cat_uint(d, 4*BRu(7, 0));
                    break;

                case 'o':
                    d = cat_uint(d, BRu(19, 16));
                    break; // for dmb, dsb, isb

                case 'h':
                    d = cat_addr(d, (pc&(~3))+4+4*BRs(7, 0));
                    break;

                case 'i':
                    {
                        int32_t s = (iw>>10)&1;
                        int32_t disp = (s << 24) // S
                                       | ((~((iw>>29) ^ s)&1) << 23) // not(J1 xor S)
                                       | ((~((iw>>27) ^ s)&1) << 22) // not(J2 xor S)
                                       | (BRu(9, 0) << 12)
                                       | ((iw & 0x7ff0000) >> 15);
                        disp = (disp << 7) >> 7; // sign extension
                        d = cat_addr(d, pc+4+disp);
                        break;
                    }

                case 'j':
                    d = cat_addr(d, pc+4+2*BRs(7, 0));
                    break;

                case 'k':
                    d = cat_addr(d, pc+4+2*BRs(10, 0));
                    break;

                case 'l':
                case 'p':
                case 'r':
                    {
                        int i;

                        for (i=0; i<8; i++)
                            if ((iw&(1<<i))!=0)
                            {
                                *d++ = 'r';
                                *d++ = '0' + i;
                                *d++ = ',';
                                *d++ = ' ';
                            }

                        if (iw & 0x100)
                        {
                            if (c=='l')
                                d = stpcpy(d, "lr, ");
                            else if (c=='p')
                                d = stpcpy(d, "pc, ");
                        }

                        d -= 2;
                        break;
                    }

                case 's':
                    {
                        switch ((iw>>16)&0xff)
                        {
                            case 0:
                                d = stpcpy(d, "apsr");
                                break;

                            case 1:
                                d = stpcpy(d, "iapsr");
                                break;

                            case 2:
                                d = stpcpy(d, "eapsr");
                                break;

                            case 3:
                                d = stpcpy(d, "xpsr");
                                break;

                            case 5:
                                d = stpcpy(d, "ipsr");
                                break;

                            case 6:
                                d = stpcpy(d, "epsr");
                                break;

                            case 7:
                                d = stpcpy(d, "iepsr");
                                break;

                            case 8:
                                d = stpcpy(d, "msp");
                                break;

                            case 9:
                                d = stpcpy(d, "psp");
                                break;

                            case 16:
                                d = stpcpy(d, "primax");
                                break;

                            case 20:
                                d = stpcpy(d, "control");
                                break;

                            default:
                                d = d + sprintf(d, "SYSm 0x%02x", (unsigned)(iw>>16)&0xff);
                        }

                        break;
                    }

                default:
                    *d++ = c;
            }
        }
        else *d++ = c;
    }
    while (c!=0);
}

#define D2(format)	disasm(d, format, pc, iw); return 2
#define D4(format)	disasm(d, format, pc, iw); return 4

int armv6m_disasm_iw(char *d, addr_t pc, uint_fast32_t iw)
{
    switch ((iw>>11) & 0x1f)
    {
        case 0x00:
            D2("lsls\t%a, %b, %5");

        case 0x01:
            D2("lsrs\t%a, %b, %6");

        case 0x02:
            D2("asrs\t%a, %b, %6");

        case 0x03:
            switch (BRu(10, 9))
            {
                case 0x00:
                    D2("adds\t%a, %b, %c");

                case 0x01:
                    D2("subs\t%a, %b, %c");

                case 0x02:
                    D2("adds\t%a, %b, %3");

                case 0x03:
                    D2("subs\t%a, %b, %3");
            }

        case 0x04:
            D2("movs\t%d, %8");

        case 0x05:
            D2("cmp\t%d, %8");

        case 0x06:
            D2("adds\t%d, %8");

        case 0x07:
            D2("subs\t%d, %8");

        case 0x08:
            switch (BRu(10, 6))
            {
                case 0x00:
                    D2("ands\t%a, %b");

                case 0x01:
                    D2("eors\t%a, %b");

                case 0x02:
                    D2("lsls\t%a, %b");

                case 0x03:
                    D2("lsrs\t%a, %b");

                case 0x04:
                    D2("asrs\t%a, %b");

                case 0x05:
                    D2("adcs\t%a, %b");

                case 0x06:
                    D2("sbcs\t%a, %b");

                case 0x07:
                    D2("ror\t%a, %b");

                case 0x08:
                    D2("tst\t%a, %b");

                case 0x09:
                    D2("rsbs\t%a, %b");

                case 0x0a:
                    D2("cmp\t%a, %b");

                case 0x0b:
                    D2("cmn\t%a, %b");

                case 0x0c:
                    D2("orrs\t%a, %b");

                case 0x0d:
                    D2("muls\t%a, %b");

                case 0x0e:
                    D2("bics\t%a, %b");

                case 0x0f:
                    D2("mvns\t%a, %b");

                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                    D2("add\t%A, %B");

                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    D2("cmp\t%A, %B");

                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b:
                    D2("mov\t%A, %B");

                case 0x1c:
                case 0x1d:
                    D2("bx\t%B");

                case 0x1e:
                case 0x1f:
                    D2("blx\t%B");
            }

        case 0x09:
            D2("ldr\t%d, [%h]");

        case 0x0a:
            switch (BRu(10, 9))
            {
                case 0x00:
                    D2("str\t%a, [%b, %c]");

                case 0x01:
                    D2("strh\t%a, [%b, %c]");

                case 0x02:
                    D2("strb\t%a, [%b, %c]");

                case 0x03:
                    D2("ldrsb\t%a, [%b, %c]");
            }

        case 0x0b:
            switch (BRu(10, 9))
            {
                case 0x00:
                    D2("ldr\t%a, [%b, %c]");

                case 0x01:
                    D2("ldrh\t%a, [%b, %c]");

                case 0x02:
                    D2("ldrb\t%a, [%b, %c]");

                case 0x03:
                    D2("ldrsh\t%a, [%b, %c]");
            }

        case 0x0c:
            D2("str\t%a, [%b, %w]");

        case 0x0d:
            D2("ldr\t%a, [%b, %w]");

        case 0x0e:
            D2("strb\t%a, [%b, %5]");

        case 0x0f:
            D2("ldrb\t%a, [%b, %5]");

        case 0x10:
            D2("strh\t%a, [%b, %W]");

        case 0x11:
            D2("ldrh\t%a, [%b, %W]");

        case 0x12:
            D2("str\t%d, [sp, %z]");

        case 0x13:
            D2("ldr\t%d, [sp, %z]");

        case 0x14:
            D2("adr\t%d, %h");

        case 0x15:
            D2("add\t%d, sp, %z");

        case 0x16:
            switch (BRu(10, 6))
            {
                case 0x00:
                case 0x01:
                    D2("add\tsp, %y");

                case 0x02:
                case 0x03:
                    D2("sub\tsp, %y");

                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    D2("rcmc\t%8");

                case 0x08:
                    D2("sxth\t%a, %b");

                case 0x09:
                    D2("sxtb\t%a, %b");

                case 0x0a:
                    D2("uxth\t%a, %b");

                case 0x0b:
                    D2("uxtb\t%a, %b");

                    // ...
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    D2("push\t{%l}");

                    // ...
                case 0x19:
                    if (iw==0xb662)
                    {
                        D2("cpsie\ti");
                    }
                    else if (iw==0xb673)
                    {
                        D2("cpsid\ti");
                    }

                    // ...
            }

            break;

        case 0x17:
            switch (BRu(10, 6))
            {
                    // ...
                case 0x08:
                    D2("rev\t%a, %b");

                case 0x09:
                    D2("rev16\t%a, %b");

                    // ...
                case 0x0b:
                    D2("revsh\t%a, %b");

                    // ...
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    D2("pop\t{%p}");

                case 0x18:
                case 0x19:
                case 0x1a:
                case 0x1b:
                    D2("bkpt\t%8");

                case 0x1c:
                    switch (BRu(5, 0))
                    {
                        case 0x00:
                            D2("nop");

                        case 0x10:
                            D2("yield");

                        case 0x20:
                            D2("wfe");

                        case 0x30:
                            D2("wfi");
                    }

                    break;

                case 0x1d:
                    if (BRu(5, 0)==0x00)
                    {
                        D2("sev");
                    }
            }

            break;

        case 0x18:
            D2("stm\t%d, {%r}");

        case 0x19:
            D2("ldm\t%d, {%r}");

        case 0x1a:
            switch (BRu(10, 8))
            {
                case 0x00:
                    D2("beq\t%j");

                case 0x01:
                    D2("bne\t%j");

                case 0x02:
                    D2("bcs\t%j");

                case 0x03:
                    D2("bcc\t%j");

                case 0x04:
                    D2("bmi\t%j");

                case 0x05:
                    D2("bpl\t%j");

                case 0x06:
                    D2("bvs\t%j");

                case 0x07:
                    D2("bvc\t%j");
            }

        case 0x1b:
            switch (BRu(10, 8))
            {
                case 0x00:
                    D2("bhi\t%j");

                case 0x01:
                    D2("bls\t%j");

                case 0x02:
                    D2("bge\t%j");

                case 0x03:
                    D2("blt\t%j");

                case 0x04:
                    D2("bgt\t%j");

                case 0x05:
                    D2("ble\t%j");

                case 0x06:
                    D2("bal\t%j");

                case 0x07:
                    D2("svc\t%8");
            }

        case 0x1c:
            D2("b\t%k");

            // 0x1d see below
        case 0x1e:
            {
                if ((iw&0xd0000000)==0xd0000000) // bl
                {
                    D4("bl\t%i");
                }
                else if ((iw&0xf000ffff)==0x8000f3ef) // mrs
                {
                    D4("mrs\t%D, %s");
                }
                else if ((iw&0xff00fff0)==0x8800f380) // msr
                {
                    D4("msr\t%s, %E");
                }
                else if ((iw&0xffff)==0xf3bf)
                {
                    switch (iw >> 20)
                    {
                        case 0x8f4:
                            D4("dsb\t%o");

                        case 0x8f5:
                            D4("dmb\t%o");

                        case 0x8f6:
                            D4("isb\t%o");
                    }

                    break;
                }
            }

            // fallthrough
        case 0x1d:
        case 0x1f:
            sprintf(d, ".word\t0x%8"PRIxFAST32, iw);
            return 4;
    }

    sprintf(d, ".half\t0x%4"PRIxFAST32, iw & 0xffff);
    return 2;
}


// Disassemble an address and return the lengh of the instruction
int armv6m_disasm(node_t *node, addr_t pc, char *dstr)
{
    uint32_t iw;
    memory_fetch_32le(node, pc, &iw);
    return armv6m_disasm_iw(dstr, pc, iw);
}


// Print a register dump
void armv6m_print_context(node_t *node)
{
    char dstr[128];
    uint32_t iw;
    memory_fetch_32le(node, node->pc, &iw);
    armv6m_disasm_iw(dstr, node->pc, iw);

    user_printf("cycle %ld  %c%c%c%c pc=%08x\t%s\n"
                "  %08x %08x %08x %08x  %08x %08x %08x %08x\n"
                "  %08x %08x %08x %08x  %08x SP=%08x LR=%08x\n",
                node->cycle,
                node->core.armv6m.apsr_n ? 'N' : '-',
                node->core.armv6m.apsr_z ? 'Z' : '-',
                node->core.armv6m.apsr_c ? 'C' : '-',
                node->core.armv6m.apsr_v ? 'V' : '-',
                node->pc, dstr,
                node->core.armv6m.reg[0],  node->core.armv6m.reg[1],  node->core.armv6m.reg[2],  node->core.armv6m.reg[3],
                node->core.armv6m.reg[4],  node->core.armv6m.reg[5],  node->core.armv6m.reg[6],  node->core.armv6m.reg[7],
                node->core.armv6m.reg[8],  node->core.armv6m.reg[9],  node->core.armv6m.reg[10], node->core.armv6m.reg[11],
                node->core.armv6m.reg[12], node->core.armv6m.reg[13], node->core.armv6m.reg[14]);
}


uint32_t armv6m_raise_exception(node_t *node, uint32_t num, uint32_t addr) {
  // PushStack(); // Reference Manual p. B1-225
  uint32_t frameptralign = 0;
  uint32_t frameptr = 0;

  //user_printf("[%d]RE: start with SP %08x\n", node->rank, REG_SP);

  // SP resides in regular register set, so we don't have to distinguish
  // sp_main/sp_process in this place!
  frameptralign = (REG_SP & (1 << 2)) >> 2;
  REG_SP = (REG_SP - 0x20) & ~0x4;
  frameptr = REG_SP;

  // ReturnAddress();
  uint32_t retAddress = 0x0;
  switch (num) {
  case (ARMV6M_EXC_SUPERVISOR_CALL):
  case (ARMV6M_EXC_EXTERNAL_IRQ):
  default:
    retAddress = node->nextpc;
  }

  //user_printf("[%d]RE: RA %08x\n", node->rank, retAddress);

  uint32_t latency = 0;
  latency += memory_store_direct(node, MA_32le, frameptr, node->core.armv6m.reg[0]);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x04, node->core.armv6m.reg[1]);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x08, node->core.armv6m.reg[2]);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x0c, node->core.armv6m.reg[3]);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x10, node->core.armv6m.reg[12]);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x14, REG_LR);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x18, retAddress);
  uint32_t psr = build_psr(node);
  psr = (psr & ~(1 << 9)) | (frameptralign << 9);
  latency += memory_store_direct(node, MA_32le, frameptr + 0x1c, psr);
  if (node->core.armv6m.mode == ARMV6M_MODE_HANDLER) {
    REG_LR = 0xfffffff1;
  }
  else {
    if (node->core.armv6m.control_spsel == false) {
      REG_LR = 0xfffffff9;
    }
    else {
      REG_LR = 0xfffffffd;
    }
  }

  // ExceptionTaken(ExceptionType);

  // should set stored registers to UNKNOWN, we'll ignore that
  if (node->core.armv6m.control_spsel && node->core.armv6m.mode == ARMV6M_MODE_THREAD) {
    // current context runs on sp_process
    //user_printf("[%d]RE: storing SP to sp_process: %08x\n", node->rank, REG_SP);
    node->core.armv6m.sp_process = REG_SP;
  }
  else {
    // current context runs on sp_main
    //user_printf("[%d]RE: storing SP to sp_main: %08x\n", node->rank, REG_SP);
    node->core.armv6m.sp_main = REG_SP;
  }
  node->core.armv6m.mode = ARMV6M_MODE_HANDLER;
  REG_SP = node->core.armv6m.sp_main;
  //user_printf("[%d]RE: loaded sp_main: %08x\n", node->rank, REG_SP);
  node->core.armv6m.ipsr = num & 0x1f;
  node->core.armv6m.control_spsel = 0;
  // ExceptionActive[ExceptionNumber] = '1';
  // SCS_UpdateStatusRegs();
  // SetEventRegister();
  // InstructionSynchronizationBarrier();
  uint32_t start = 0;
  latency += memory_load_u32_direct(node, MA_32le, ARMV6M_VECTORTABLE + 4 * num + 4, &start);
  node->nextpc = start & ~0x1;
  return latency;
}


uint32_t armv6m_leave_exception(node_t *node, uint32_t exc_return) {
  uint32_t latency = 0;
  // ExceptionReturn(); B1-229 f.
  assert(node->core.armv6m.mode == ARMV6M_MODE_HANDLER);
  if ( (exc_return & ARMV6M_EXC_RETURN_MASK) != ARMV6M_EXC_RETURN_MASK) {
    // error!
    return -1;
  }
  //uint32_t returningExceptionNumber = node->core.armv6m.ipsr & 0x1f;
  //uint32_t nestedActivation = 0;
  // we assume no nested exceptions
  // nestedActivation = ExceptionActiveBitCount();
  /*
  int i = 0;
  for (i=0; i<5; i++) {
    if ( ??? & (1 << i) ) {
      ++nestedActivation;
    }
  }
  */
  //user_printf("[%d]LE: start with SP %08x\n", node->rank, REG_SP);
  uint32_t frameptr = 0;
  //uint32_t currentMode = 0;
  switch (exc_return & 0xf) {
  case 0x1: // exception occurred during handler mode
    // if (nestedActivation == 1) UNPREDICTABLE
    frameptr = REG_SP;
    node->core.armv6m.mode = ARMV6M_MODE_HANDLER;
    node->core.armv6m.control_spsel = 0;
    //user_printf("[%d]LE: Using sp_main as frameptr (1): 0x%08x\n", node->rank, frameptr);
    break;

  case 0x9: // exception occurred in thread mode, using sp_main
    // if (nestedActivation != 1) UNPREDICTABLE
    frameptr = REG_SP;
    node->core.armv6m.mode = ARMV6M_MODE_THREAD;
    node->core.armv6m.control_spsel = 0;
    //user_printf("[%d]LE: Using sp_main as frameptr (2): 0x%08x\n", node->rank, frameptr);
    break;

  case 0xd: // exception occurred in thread mode, using sp_process
    // switch back to sp_process
    node->core.armv6m.sp_main = REG_SP;
    REG_SP = node->core.armv6m.sp_process;
    frameptr = REG_SP;
    node->core.armv6m.mode = ARMV6M_MODE_THREAD;
    node->core.armv6m.control_spsel = 1;
    //user_printf("[%d]LE: Using sp_process as frameptr: 0x%08x\n", node->rank, frameptr);
    break;

  default:
    // UNPREDICTABLE
    break;
  }


  // DeActivate(returningExceptionNumber); B1-230 f.
  // ExceptionActiver[returningExceptionNumber] = 0;

  // PopStack(); B1-230 f.

  uint32_t psr;
  uint32_t npc;
  latency += memory_load_u32_direct(node, MA_32le, frameptr, &node->core.armv6m.reg[0]);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x04, &node->core.armv6m.reg[1]);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x08, &node->core.armv6m.reg[2]);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x0c, &node->core.armv6m.reg[3]);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x10, &node->core.armv6m.reg[12]);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x14, &REG_LR);
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x18, &npc);
  node->nextpc = npc;
  latency += memory_load_u32_direct(node, MA_32le, frameptr+0x1c, &psr);

  uint32_t psr9x = ((psr >> 9) & 0x1) << 2;

  //user_printf("[%d]LE: RA %08x\n", node->rank, npc);
  //user_printf("[%d]LE: R0 %08x\n", node->rank, node->core.armv6m.reg[0]);
  //user_printf("[%d]LE: R1 %08x\n", node->rank, node->core.armv6m.reg[1]);

  /*
  switch (exc_return & 0xf) {
  case 0x1: // Returning to handler mode
    node->core.armv6m.sp_main = (node->core.armv6m.sp_main + 0x20) | psr9x;
    break;

  case 0x9: // Returning to thread mode using main stack
    node->core.armv6m.sp_main = (node->core.armv6m.sp_main + 0x20) | psr9x;
    break;

  case 0xd: // Returning to thread mode using process stack
    node->core.armv6m.sp_process = (node->core.armv6m.sp_process + 0x20) | psr9x;
    break;
  }
  */
  REG_SP = (REG_SP + 0x20) | psr9x; // SP was already switched in upper switch/case!
  //user_printf("[%d]LE: finishes with SP %08x\n", node->rank, REG_SP);

  restore_apsr(node, psr);
  bool force_thread = (node->core.armv6m.mode == ARMV6M_MODE_THREAD) && (node->core.armv6m.control_npriv == true);
  node->core.armv6m.ipsr = force_thread ? 0x00 : (psr & 0x1f);
  // EPSR<24> = psr<24>;


  if (node->core.armv6m.mode == ARMV6M_MODE_HANDLER) {
    if ( (node->core.armv6m.ipsr & 0x1f) == 0 ) {
      // UNPREDICTABLE
    }
  }
  else {
    if ( (node->core.armv6m.ipsr & 0x1f) != 0 ) {
      // UNPREDICTABLE
    }
  }

  // SetEventRegister();
  // InstructionSznchronizationBarrier();

  // if.... SleepOnExit();

  return latency;
}
