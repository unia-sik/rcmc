/**
 * riscv.c
 * RISC-V ISA
 *
 * MacSim project
 *
 * TODO:
 *      fp exceptions
 *      fclass
 *      NOCDIM
 */

#include "riscv.h"
#include "memory.h"
#include <fenv.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>




#define CUT_TO_RANGE(__min, __ll, __max) \
    ((__ll<__min) ? __min : ((__ll>__max) ? __max : __ll))





#define RISCV_STACK_END                 0xF0000000
#define RISCV_STACK_ALIGN_MASK          (-64)

// latency of instructions (in clock cycles)
#define RISCV_LATENCY_ARITH             1
#define RISCV_LATENCY_ADDR_CALC         0
#define RISCV_LATENCY_INTERLOCK         1
#define RISCV_LATENCY_MUL               5
#define RISCV_LATENCY_DIV32             35
#define RISCV_LATENCY_DIV64             67
#define RISCV_LATENCY_BRANCH_NOTTAKEN   1
#define RISCV_LATENCY_BRANCH_TAKEN      3
#define RISCV_LATENCY_FENCE             1
#define RISCV_LATENCY_EXCEPTION         3
#define RISCV_LATENCY_CSR               1
#define RISCV_LATENCY_JAL               3
#define RISCV_LATENCY_JALR              3

#define RISCV_LATENCY_FMIN              1
#define RISCV_LATENCY_FCMP              1
#define RISCV_LATENCY_FMV               1
#define RISCV_LATENCY_F2I               2
#define RISCV_LATENCY_I2F               2
#define RISCV_LATENCY_FADD              3
#define RISCV_LATENCY_FMUL              3
#define RISCV_LATENCY_FMA               3
#define RISCV_LATENCY_FDIV_S            25
#define RISCV_LATENCY_FDIV_D            31
#define RISCV_LATENCY_FSQRT_S           28
#define RISCV_LATENCY_FSQRT_D           37

// core special registers
#define RISCV_CSR_FFLAGS        0x001
#define RISCV_CSR_FRM           0x002
#define RISCV_CSR_FCSR          0x003 // rw
#define RISCV_CSR_CYCLE         0xc00 // ro
#define RISCV_CSR_TIME          0xc01 // ro
#define RISCV_CSR_INSTRET       0xc02 // ro
#define RISCV_CSR_CYCLEH        0xc80 // =0
#define RISCV_CSR_TIMEH         0xc81 // =0
#define RISCV_CSR_INSTRETH      0xc82 // =0

#define RISCV_CSR_MCPUID        0xf00 // ro
#define RISCV_CSR_MIMPID        0xf01 // ro
#define RISCV_CSR_MHARTID       0xf10 // ro
#define RISCV_CSR_MSTATUS       0x300 // rw implementation required!
#define RISCV_CSR_MTVEC         0x301 // rw
#define RISCV_CSR_MTDELEG       0x302
#define RISCV_CSR_MIE           0x304
#define RISCV_CSR_MTIMECMP      0x321
#define RISCV_CSR_MTIME         0x701
#define RISCV_CSR_MTIMEH        0x741 // =0
#define RISCV_CSR_MSCRATCH      0x340 // rw
#define RISCV_CSR_MEPC          0x341 // rw
#define RISCV_CSR_MCAUSE        0x342 // rw
#define RISCV_CSR_MBADADDR      0x343
#define RISCV_CSR_MIP           0x344
#define RISCV_CSR_MBASE         0x380
#define RISCV_CSR_MBOUND        0x381
#define RISCV_CSR_MIBASE        0x382
#define RISCV_CSR_MIBOUND       0x383
#define RISCV_CSR_MDBASE        0x384
#define RISCV_CSR_MDBOUND       0x385
#define RISCV_CSR_HTIMEW        0xb01
#define RISCV_CSR_HTIMEHW       0xb81
#define RISCV_CSR_MTOHOST       0x780
#define RISCV_CSR_MFROMHOST     0x781

// RC/MC extension
#define RISCV_CSR_PUTCHAR       0x782
#define RISCV_CSR_MAXCID        0xC70 // ro
#define RISCV_CSR_CID           0xC71 // ro == MHARTID
#define RISCV_CSR_NOCDIM        0xC72 // ro implementation required!
#define RISCV_CSR_SENDRDY       0xC73 // ro
#define RISCV_CSR_NEXTRECV      0xC74 // ro

// floating point exception flags for fcsr
#define RISCV_FFLAGS_NX         1
#define RISCV_FFLAGS_UF         2
#define RISCV_FFLAGS_OF         4
#define RISCV_FFLAGS_DZ         8
#define RISCV_FFLAGS_NV         16

// result of fclass
#define RISCV_FCLASS_NEG_INF            0x0001
#define RISCV_FCLASS_NEG_NORMAL         0x0002
#define RISCV_FCLASS_NEG_SUB            0x0004
#define RISCV_FCLASS_NEG_ZERO           0x0008
#define RISCV_FCLASS_POS_ZERO           0x0010
#define RISCV_FCLASS_POS_SUB            0x0020
#define RISCV_FCLASS_POS_NORMAL         0x0040
#define RISCV_FCLASS_POS_INF            0x0080
#define RISCV_FCLASS_SIGNALING_NAN      0x0100
#define RISCV_FCLASS_QUIET_NAN          0x0200

#define RISCV_CANONICAL_NAN_FLOAT       0x7FC00000
#define RISCV_CANONICAL_NAN_DOUBLE      0x7FF8000000000000


// cause of exceptions
#define RISCV_EXCEPTION_MISALIGNED_INSTRUCTION          0
#define RISCV_EXCEPTION_INSTRUCTION_ACCESS_FAULT        1
#define RISCV_EXCEPTION_ILLEGAL_INSTRUCTION             2
#define RISCV_EXCEPTION_ECALL                           6
#define RISCV_EXCEPTION_EBREAK                          7
#define RISCV_EXCEPTION_MISALIGNED_LOAD                 8
#define RISCV_EXCEPTION_MISALIGNED_STORE                9

#define RISCV_EXCEPTION_EXTERNAL_INTERRUPT              0x8000000000000002



#define CHECK_ALIGNMENT(addr, mask) \
     if (addr&mask) warning(0, "unaligned memory access in cycle %llu", node->cycle)




// Check if the CSR number is valid.
// For some CSRs only reading is allowed. Check this, too.
static inline bool csr_valid(uint_fast32_t iw)
{
    unsigned no = (iw>>20) & 0xfff;
    switch (no) {
    case RISCV_CSR_CYCLE:
    case RISCV_CSR_TIME:
    case RISCV_CSR_INSTRET:
    case RISCV_CSR_CYCLEH:
    case RISCV_CSR_TIMEH:
    case RISCV_CSR_INSTRETH:

    case RISCV_CSR_MCPUID:
    case RISCV_CSR_MIMPID:
    case RISCV_CSR_MHARTID:

    case RISCV_CSR_MAXCID:
    case RISCV_CSR_CID:
    case RISCV_CSR_NOCDIM:
    case RISCV_CSR_SENDRDY:
    case RISCV_CSR_NEXTRECV:
        if ((iw&0x000ff000)==0x00002000) // rs1==0, funct3=2
            return true;

    case RISCV_CSR_FFLAGS:
    case RISCV_CSR_FRM:
    case RISCV_CSR_FCSR:

    case RISCV_CSR_MSTATUS:
    case RISCV_CSR_MTVEC:
    case RISCV_CSR_MSCRATCH:
    case RISCV_CSR_MEPC:
    case RISCV_CSR_MCAUSE:
    case RISCV_CSR_MTOHOST:

    case RISCV_CSR_PUTCHAR:
        return true;
    }
    return false;
}


static int64_t csr_read(node_t *node, uint_fast16_t no)
{
    switch (no) {
    case RISCV_CSR_FFLAGS:      return node->core.riscv.fcsr & 0x1f;
    case RISCV_CSR_FRM:         return (node->core.riscv.fcsr>>5) & 7;
    case RISCV_CSR_FCSR:        return node->core.riscv.fcsr;
    case RISCV_CSR_CYCLE:
    case RISCV_CSR_TIME:        return node->cycle;
    case RISCV_CSR_INSTRET:     return node->retired;
    case RISCV_CSR_CYCLEH:
    case RISCV_CSR_TIMEH:
    case RISCV_CSR_INSTRETH:    return 0;

    case RISCV_CSR_MCPUID:      return 0x8000000001000100; // RV64IX
    case RISCV_CSR_MIMPID:      return 0x2cdc;
    case RISCV_CSR_MHARTID:     return node->rank;
    case RISCV_CSR_MSTATUS:     return node->core.riscv.mstatus;
    case RISCV_CSR_MTVEC:       return node->core.riscv.mtvec;
    case RISCV_CSR_MSCRATCH:    return node->core.riscv.mscratch;
    case RISCV_CSR_MEPC:        return node->core.riscv.mepc;
    case RISCV_CSR_MCAUSE:      return node->core.riscv.mcause;

    case RISCV_CSR_MTOHOST:     return 0;
    case RISCV_CSR_PUTCHAR:     return 0;
    case RISCV_CSR_MAXCID:      return conf_max_rank;
    case RISCV_CSR_CID:         return node->rank;
    case RISCV_CSR_NOCDIM:      return (0x0001000100000000 |
                                       (conf_noc_height<<16) | conf_noc_width);
    case RISCV_CSR_SENDRDY:     return node->noc_sender_ready(node) ? 0 : 1;
    case RISCV_CSR_NEXTRECV:    return node->noc_probe_any(node);
    }
    return 0;
}


static void csr_write(node_t *node, uint_fast16_t no, int64_t value)
{
    switch (no) {
    case RISCV_CSR_FFLAGS:      node->core.riscv.fcsr = 
        (node->core.riscv.fcsr & ~0x1f) | (value & 0x1f); break;
    case RISCV_CSR_FRM:         node->core.riscv.fcsr = 
        (node->core.riscv.fcsr & ~0xe0) | ((value&7)<<5); break;
    case RISCV_CSR_FCSR:        node->core.riscv.fcsr = value & 0xff; break;

    case RISCV_CSR_MSTATUS:     node->core.riscv.mstatus = value; break;
    case RISCV_CSR_MTVEC:       node->core.riscv.mtvec = value; break;
    case RISCV_CSR_MSCRATCH:    node->core.riscv.mscratch = value; break;
    case RISCV_CSR_MEPC:        node->core.riscv.mepc = value; break;
    case RISCV_CSR_MCAUSE:      node->core.riscv.mcause = value; break;

    case RISCV_CSR_PUTCHAR:     core_printf(node->rank, "%c", value); break;
    }
}


static inline instruction_class_t ic_branch(node_t *node, bool cond, addr_t disp)
{
    if (cond) {
        node->nextpc = node->pc + disp;
        return RISCV_LATENCY_BRANCH_TAKEN;
    }
    return RISCV_LATENCY_BRANCH_NOTTAKEN;
}


static inline instruction_class_t ic_exception(node_t *node, uint64_t cause)
{
    node->core.riscv.mcause = cause;
    node->core.riscv.mepc = node->pc;
    node->nextpc = node->core.riscv.mtvec;
    return RISCV_LATENCY_EXCEPTION;
}



// ---------------------------------------------------------------------
// floating point math
// ---------------------------------------------------------------------


// prepare the host FPU environment before a client instruction is emulated
static void prepare_host_fp_env()
{
    feclearexcept(FE_ALL_EXCEPT);
}


// set the RISC-V flags according to the hosts FPU environment
static void update_client_fp_env(node_t *node)
{
    int raised = fetestexcept(FE_ALL_EXCEPT);
    node->core.riscv.fcsr = node->core.riscv.fcsr
        | ((raised&FE_INEXACT)   ? RISCV_FFLAGS_NX : 0)
        | ((raised&FE_DIVBYZERO) ? RISCV_FFLAGS_DZ : 0)
        | ((raised&FE_UNDERFLOW) ? RISCV_FFLAGS_UF : 0)
        | ((raised&FE_OVERFLOW)  ? RISCV_FFLAGS_OF : 0)
        | ((raised&FE_INVALID)   ? RISCV_FFLAGS_NV : 0);
}


static void set_rm(node_t *node, uint_fast32_t iw)
{
    prepare_host_fp_env();
    switch (iw&0x7000) {
    case 0x0000: fesetround(FE_TONEAREST); break;
    case 0x1000: fesetround(FE_TOWARDZERO); break;
    case 0x2000: fesetround(FE_DOWNWARD); break;
    case 0x3000: fesetround(FE_UPWARD); break;
//    case 0x4000: fsetround(FE_RMM); break;
    case 0x7000:
        switch (node->core.riscv.fcsr & 0xe0) {
        case 0x00: fesetround(FE_TONEAREST); break;
        case 0x20: fesetround(FE_TOWARDZERO); break;
        case 0x40: fesetround(FE_DOWNWARD); break;
        case 0x60: fesetround(FE_UPWARD); break;
//        case 0x80: fsetround(FE_RMM); break;
        default:
            fatal("invalid rounding mode in frm");
        }
        break;
    default:
        fatal("invalid rounding mode in instruction");
    }
}


static int issignaling_float(float f)
{
    uf32_t x;
    x.f = f;
    return (((x.u & 0x7FC00000) == 0x7F800000) &&
            ((x.u & 0x003FFFFF) != 0)) ? 1 : 0;
}


static int issignaling_double(double f)
{
    uf64_t x;
    x.f = f;
    return (((x.u & 0x7FF8000000000000) == 0x7FF0000000000000) &&
            ((x.u & 0x0007FFFFFFFFFFFF) != 0)) ? 1 : 0;
}


static inline float unboxing_float(double d)
{
    uf64_t x;
    uf32_t y;
    x.f = d;
    if ((x.i>>32) != -1) {
        y.u = RISCV_CANONICAL_NAN_FLOAT;
    } else {
        y.u = x.u;
    }
    return y.f;
}


static inline double boxing_float(float f)
{
    uf32_t x;
    uf64_t y;
    x.f = f;
    y.u = 0xffffffff00000000 | x.u;
    return y.f;
}


static inline double canonical_nan_float(float f)
{
    uf64_t x;
    if (isnan(f)) {
        x.u = 0xffffffff00000000 | RISCV_CANONICAL_NAN_FLOAT;
    } else {
        x.f = boxing_float(f);
    }
    return x.f;
}


static inline double canonical_nan_double(double f)
{
    uf64_t x;
    x.f = f;
    if (isnan(x.f)) x.u = RISCV_CANONICAL_NAN_DOUBLE;
    return x.f;
}


static uint_fast16_t fclass_float(float f)
{
    uf32_t x;
    x.f = f;
    unsigned s = x.u>>31;
    unsigned e = (x.u>>23) & 0xFF;

    if (e==0) {
        if ((x.u<<1)==0) return s ? RISCV_FCLASS_NEG_ZERO : RISCV_FCLASS_POS_ZERO;
        return s ? RISCV_FCLASS_NEG_SUB : RISCV_FCLASS_POS_SUB;
    }
    if (e==0xFF) {
        if ((x.u<<9)==0) return s ? RISCV_FCLASS_NEG_INF : RISCV_FCLASS_POS_INF;
        return ((x.u>>22)&1)==0 ? RISCV_FCLASS_SIGNALING_NAN : RISCV_FCLASS_QUIET_NAN;
    }
    return s ? RISCV_FCLASS_NEG_NORMAL : RISCV_FCLASS_POS_NORMAL;
}


static uint_fast16_t fclass_double(double f)
{
    uf64_t x;
    x.f = f;
    unsigned s = x.u>>63;
    unsigned e = (x.u>>52) & 0x7FF;
    if (e==0) {
        if ((x.u<<1)==0) return s ? RISCV_FCLASS_NEG_ZERO : RISCV_FCLASS_POS_ZERO;
        return s ? RISCV_FCLASS_NEG_SUB : RISCV_FCLASS_POS_SUB;
    }
    if (e==0x7FF) {
        if ((x.u<<12)==0) return s ? RISCV_FCLASS_NEG_INF : RISCV_FCLASS_POS_INF;
        return ((x.u>>51)&1)==0 ? RISCV_FCLASS_SIGNALING_NAN : RISCV_FCLASS_QUIET_NAN;
    }
    return s ? RISCV_FCLASS_NEG_NORMAL : RISCV_FCLASS_POS_NORMAL;
}


static inline uint64_t riscv_fcvt(node_t *node, double f, int64_t min, uint64_t max)
{
    long long ll; // C99: at least 64 bit wide

    if (!isfinite(f)) {
        node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
        return (isinf(f) && signbit(f)) ? min : max;
            // min for -inf, max for +inf or NaN
    }

    // special case, because there is no unsigned llrintf()
    if (max==UINT64_MAX) {
        if (f < ldexp(1.0, 62)) {
            ll = llrintf(f);
            update_client_fp_env(node);
            if (ll >= min) return ll;
            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
            return min;
        } 
        if (f >= ldexp(1.0, 64)) {
            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
            return UINT64_MAX;
        }
        ll = 4*llrintf(f/4.0); 
            // no subtraction to avoid adjusting the rounding mode
        update_client_fp_env(node);
        return ll;
    }

    ll = llrintf(f);
    if (fetestexcept(FE_INVALID)) {
        // result of llrintf() is undefined if invalid exception
        node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
        return (f <= min) ? min : max;
    }
    update_client_fp_env(node);

    if (ll < min) {
        node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
        return min;
    } else if (ll > (int64_t)max) {
        node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
        return max;
    }
    return ll;
}

static inline uint64_t riscv_mulhu(uint64_t a, uint64_t b)
{
    uint64_t a0 = (uint32_t)a;
    uint64_t a1 = a >> 32;
    uint64_t b0 = (uint32_t)b;
    uint64_t b1 = b >> 32;
    uint64_t t = a1*b0 + ((a0*b0) >> 32);
    return a1*b1 + (t>>32) + ((a0*b1 + (t&0xffffffff)) >> 32);
}

static inline uint64_t riscv_mulhsu(int64_t a, uint64_t b)
{
    return (a<0)
        ? (~riscv_mulhu(-a, b) + (a*b==0))
        : riscv_mulhu(a, b);
}

static inline uint64_t riscv_mulh(int64_t a, int64_t b)
{
    return (b<0) 
        ? riscv_mulhsu(-a, -b)
        : riscv_mulhsu(a, b);
}



#define BRu(h,l)	(((iw)>>(l))&((1<<((h)-(l)+1))-1))
#define BRs(h,l)	((((int64_t)(iw))<<(63-(h)))>>(63-(h)+(l)))

#define REG_D           node->core.riscv.reg[BRu(11, 7)]
#define REG_S           node->core.riscv.reg[BRu(19, 15)]
#define REG_T           node->core.riscv.reg[BRu(24, 20)]
#define REG_Du          (*((uint64_t *)&node->core.riscv.reg[BRu(11, 7)]))
#define REG_Su          (*((uint64_t *)&node->core.riscv.reg[BRu(19, 15)]))
#define REG_Tu          (*((uint64_t *)&node->core.riscv.reg[BRu(24, 20)]))
//#define WREG_D          (*((int32_t *)&node->core.riscv.reg[BRu(11, 7)]))
#define WREG_S          (*((int32_t *)&node->core.riscv.reg[BRu(19, 15)]))
#define WREG_T          (*((int32_t *)&node->core.riscv.reg[BRu(24, 20)]))
//#define WREG_Du         (*((uint32_t *)&node->core.riscv.reg[BRu(11, 7)]))
#define WREG_Su         (*((uint32_t *)&node->core.riscv.reg[BRu(19, 15)]))
#define WREG_Tu         (*((uint32_t *)&node->core.riscv.reg[BRu(24, 20)]))

#define DREG_D          node->core.riscv.freg[BRu(11, 7)]
#define DREG_S          node->core.riscv.freg[BRu(19, 15)]
#define DREG_T          node->core.riscv.freg[BRu(24, 20)]
#define DREG_U          node->core.riscv.freg[BRu(31, 27)]
#define SREG_S          unboxing_float(DREG_S)
#define SREG_T          unboxing_float(DREG_T)
#define SREG_U          unboxing_float(DREG_U)


#define IMM_B           ((((int64_t)(int32_t)iw>>19)&0xfffffffffffff000) \
                        | ((iw<<4) &0x00000800) \
                        | ((iw>>20)&0x000007e0) \
                        | ((iw>>7) &0x0000001e))
#define IMM_I           ((int64_t)(int32_t)iw>>20)
#define IMM_SHAMT       ((iw>>20)&0x3f)
#define IMM_J           ((((int64_t)(int32_t)iw>>11)&0xfffffffffff00000) \
                        | ((iw)    &0x000ff000) \
                        | ((iw>>9) &0x00000800) \
                        | ((iw>>20)&0x000007fe))
#define IMM_S           ((((int64_t)(int32_t)iw>>20)&0xffffffffffffffe0) \
                        | ((iw>>7) &0x0000001f))
#define IMM_U           (((int64_t)(int32_t)iw>>12)<<12)
#define IMM_Z           ((iw>>15)&0x1f)

#define MASK_FUNCT3     0x7000

#define nBRu(h,l)	(((next_iw)>>(l))&((1<<((h)-(l)+1))-1))




bool riscv_instruction_uses_reg_s(uint_fast32_t iw) {
  switch (iw & 0x7f) {
    case 0x0f:  // FENCE
      return false;
    case 0x17:  // AUIPC
      return false;
    case 0x37:  // LUI
      return false;
    case 0x6b:  // RC/MC
      if ((iw & 0x7000) != 0) {
        //
        switch (iw & 0x7000) {
          case 0x1000:  // Cong
            return false;
          case 0x6000:  // Wait
            return false;
          case 0x7000:  // Any
            return false;
          default:
            break;
        }
      }
      break;
    case 0x6F:  // JAL
      return false;
    case 0x73:  // SYSTEM
      if ((iw & 0x7000) != 0) {
        if (csr_valid(iw)) {
          // IMM csr ops do not use reg_s
          switch (iw & 0x7000) {
            case 0x5000:
              return false;
            case 0x6000:
              return false;
            case 0x7000:
              return false;
            default:
              break;
          }
        }
      }
      break;
    default:
      break;
  }
  return true;
}

bool riscv_instruction_uses_reg_t(uint_fast32_t iw) {
  switch (iw & 0x7f) {
    case 0x23:  // Store
      return true;
    case 0x33:  // OP reg, reg
      return true;
    case 0x3b:  // OP32 reg, reg
      return true;
    case 0x63:  // BRANCH
      return true;
    case 0x6b:  // RC/MC
      if ((iw & 0x7000) == 0) { // Send s, t
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool riscv_instruction_uses_dreg_s(uint_fast32_t iw) {
  switch (iw & 0x7f) {
    case 0x43:  // FMADD
      return true;
    case 0x47:  // FMSUB
      return true;
    case 0x4b:  // FNMSUB
      return true;
    case 0x4f:  // FNMADD
      return true;
    case 0x53:  // FP Operation
      switch ((iw>>25)&0x7f) {
        case 0x00: // fadd.s
            return true;
        case 0x01: // fadd.d
            return true;
        case 0x04: // fsub.s
            return true;
        case 0x05: // fsub.d
            return true;
        case 0x08: // fmul.s
            return true;
        case 0x09: // fmul.d
            return true;
        case 0x0c: // fdiv.s
            return true;
        case 0x0d: // fdiv.d
            return true;
        case 0x10:
            switch (iw&0x7000){
                case 0x0000: // fsgnj.s
                    return true;
                case 0x1000: // fsgnjn.s
                    return true;
                case 0x2000: // fsgnjx.s
                    return true;
            }
            break;
        case 0x11:
            switch (iw&0x7000) {
                case 0x0000: // fsgnj.d
                    return true;
                case 0x1000: // fsgnjn.d
                    return true;
                case 0x2000: // fsgnjx.d
                    return true;
            }
            break;
        case 0x14:
            switch (iw&0x7000) {
            case 0x0000: // fmin.s
                return true;
            case 0x1000: // fmax.s
                return true;
            }
            break;
        case 0x15:
            switch (iw&0x7000) {
            case 0x0000: // fmin.d
                return true;
            case 0x1000: // fmax.d
                return true;
            }
            break;


        case 0x20: 
            if ((iw&0x01f00000)==0x00100000) { // fcvt.s.d
                return true;
            }
            break;
        case 0x21: 
            if ((iw&0x01f00000)==0x00000000) { // fcvt.d.s
                return true;
            }
            break;
        case 0x2c: 
            if ((iw&0x01f00000)==0) { // fsqrt.s
                return true;
            }
            break;
        case 0x2d: 
            if ((iw&0x01f00000)==0) { // fsqrt.d
                return true;
            }
            break;
        case 0x50:
            switch (iw&0x7000) {
                case 0x0000: // fle.s
                    return true;
                case 0x1000: // flt.s
                    return true;
                case 0x2000: // feq.s
                    return true;
            }
            break;
        case 0x51:
            switch (iw&0x7000) {
                case 0x0000: // fle.s
                    return true;
                case 0x1000: // flt.s
                    return true;
                case 0x2000: // feq.s
                    return true;
            }
            break;

        case 0x60:
            switch ((iw>>20)&0x1f) {
            case 0x00: // fcvt.w.s
                return true;
            case 0x01: // fcvt.wu.s
                return true;
            case 0x02: // fcvt.l.s
                return true;
            case 0x03: // fcvt.lu.s
                return true;
            }
            break;
        case 0x61:
            switch ((iw>>20)&0x1f) {
            case 0x00: // fcvt.w.d
               return true;
            case 0x01: // fcvt.wu.d
                return true;
            case 0x02: // fcvt.l.d
                return true;
            case 0x03: // fcvt.lu.d
                return true;
            }
            break;
        case 0x70:
            if ((iw&0x01f07000)==0x0000) { // fmv.x.w
                return true;
            } else if ((iw&0x01f07000)==0x1000) { // fclass.s
                return true;
            }
            break;
        case 0x71:
            if ((iw&0x01f07000)==0) { // fmv.x.d
                return true;
            } else if ((iw&0x01f07000)==0x1000) { // fclass.d
                return true;
            }
            break;
        default:
            break;
        }
        break;
    
    default:
      break;
  }
  return false;
}

bool riscv_instruction_uses_dreg_t(uint_fast32_t iw) {
  switch (iw & 0x7f) {
    case 0x27:  // FPU Store
      return true;
    case 0x43:  // FMADD
      return true;
    case 0x47:  // FMSUB
      return true;
    case 0x4b:  // FNMSUB
      return true;
    case 0x4f:  // FNMADD
      return true;
    case 0x53:  // FP Operation
      switch ((iw>>25)&0x7f) {
        case 0x00: // fadd.s
            return true;
        case 0x01: // fadd.d
            return true;
        case 0x04: // fsub.s
            return true;
        case 0x05: // fsub.d
            return true;
        case 0x08: // fmul.s
            return true;
        case 0x09: // fmul.d
            return true;
        case 0x0c: // fdiv.s
            return true;
        case 0x0d: // fdiv.d
            return true;
        case 0x10:
            switch (iw&0x7000){
                case 0x0000: // fsgnj.s
                    return true;
                case 0x1000: // fsgnjn.s
                    return true;
                case 0x2000: // fsgnjx.s
                    return true;
            }
            break;
        case 0x11:
            switch (iw&0x7000) {
                case 0x0000: // fsgnj.d
                    return true;
                case 0x1000: // fsgnjn.d
                    return true;
                case 0x2000: // fsgnjx.d
                    return true;
            }
            break;
        case 0x14:
            switch (iw&0x7000) {
            case 0x0000: // fmin.s
                return true;
            case 0x1000: // fmax.s
                return true;
            }
            break;
        case 0x15:
            switch (iw&0x7000) {
            case 0x0000: // fmin.d
                return true;
            case 0x1000: // fmax.d
                return true;
            }
            break;
        case 0x50:
            switch (iw&0x7000) {
                case 0x0000: // fle.s
                    return true;
                case 0x1000: // flt.s
                    return true;
                case 0x2000: // feq.s
                    return true;
            }
            break;
        case 0x51:
            switch (iw&0x7000) {
                case 0x0000: // fle.s
                    return true;
                case 0x1000: // flt.s
                    return true;
                case 0x2000: // feq.s
                    return true;
            }
            break;
        default:
            break;
        }
        break;
    
    default:
      break;
  }
  return false;
}

bool riscv_instruction_uses_dreg_u(uint_fast32_t iw) {
  switch (iw & 0x7f) {
    case 0x43:  // FMADD
      return true;
    case 0x47:  // FMSUB
      return true;
    case 0x4b:  // FNMSUB
      return true;
    case 0x4f:  // FNMADD
      return true;
    default:
      break;
  }
  return false;
}

// ---------------------------------------------------------------------
// Main switch for decoding and execution
// ---------------------------------------------------------------------


instruction_class_t riscv_execute_iw(node_t *node, uint_fast32_t iw, uint_fast32_t next_iw)
{
    uf32_t x32, y32;
    uf64_t x64, y64;

    // Originally only used if the instruction is unknown.
    // But for the register dump generation we use it to recognise branches.
    node->instruction_word = iw;

    node->core.riscv.reg[0] = 0;

    switch (iw & 0x7f)
    {
        case 0x37: // lui
            REG_D = IMM_U;
            return RISCV_LATENCY_ARITH;

        case 0x17: // auipc
            REG_D = node->pc + IMM_U;
            return RISCV_LATENCY_ARITH;

        case 0x6f: // jal
            node->nextpc = node->pc + IMM_J;
            REG_D = node->pc + 4;
            return RISCV_LATENCY_JAL;

        case 0x67: // jalr
            if ((iw & 0x7000)!=0) break; // unknown if funct3!=0
            node->nextpc = REG_S + IMM_I;
            REG_D = node->pc + 4;
            return RISCV_LATENCY_JALR;

        case 0x63:
        {
            addr_t disp = IMM_B;
            switch (iw & 0x7000) {
            case 0x0000: return ic_branch(node, REG_S==REG_T, disp); // beq
            case 0x1000: return ic_branch(node, REG_S!=REG_T, disp); // bne
            case 0x4000: return ic_branch(node, REG_S<REG_T, disp); // blt
            case 0x5000: return ic_branch(node, REG_S>=REG_T, disp); // bge
            case 0x6000: return ic_branch(node, REG_Su<REG_Tu, disp); // bltu
            case 0x7000: return ic_branch(node, REG_Su>=REG_Tu, disp); // bgeu
            // invalid if funct3==2 or funct3==3
            }
            break;
        }

        case 0x03:
        {
            addr_t addr = REG_S + IMM_I;
            instruction_class_t latency = RISCV_LATENCY_ADDR_CALC;

            // next_instruction.REG_S = REG_D and next_instruction uses REG_S
            if (riscv_instruction_uses_reg_s(next_iw) && nBRu(19, 15) == BRu(11, 7)) {
                latency = RISCV_LATENCY_INTERLOCK;
            }
            // next_instruction.REG_T = REG_D and next_instruction uses REG_T
            if (riscv_instruction_uses_reg_t(next_iw) && nBRu(24, 20) == BRu(11, 7)) {
                latency = RISCV_LATENCY_INTERLOCK;
            }

            switch (iw & 0x7000) {
            case 0x0000: 
                return latency
                + generic_memory_load_u64(node, MA_8, addr, &REG_Du); // lb
            case 0x1000: 
                CHECK_ALIGNMENT(addr, 1);
                return latency
                + generic_memory_load_u64(node, MA_16le, addr, &REG_Du); // lh
            case 0x2000: 
                CHECK_ALIGNMENT(addr, 3);
                return latency
                + generic_memory_load_u64(node, MA_32le, addr, &REG_Du); // lw
            case 0x3000:
                CHECK_ALIGNMENT(addr, 7);
                return latency
                + generic_memory_load_u64(node, MA_64le, addr, &REG_Du); // ld
            case 0x4000:
                return latency
                + generic_memory_load_u64(node, MA_u8, addr, &REG_Du); // lbu
            case 0x5000:
                CHECK_ALIGNMENT(addr, 1);
                return latency
                + generic_memory_load_u64(node, MA_u16le, addr, &REG_Du); // lhu
            case 0x6000:
                CHECK_ALIGNMENT(addr, 3);
                return latency
                + generic_memory_load_u64(node, MA_u32le, addr, &REG_Du); // lwu
            }
            break;
        }

        case 0x23:
        {
            addr_t addr = REG_S + IMM_S;
            switch (iw & 0x7000) {
            case 0x0000: 
                return RISCV_LATENCY_ADDR_CALC
                + generic_memory_store(node, MA_8, addr, REG_Tu); // sb
            case 0x1000:
                CHECK_ALIGNMENT(addr, 1);
                return RISCV_LATENCY_ADDR_CALC
                + generic_memory_store(node, MA_16le, addr, REG_Tu); // sh
            case 0x2000:
                CHECK_ALIGNMENT(addr, 3);
                return RISCV_LATENCY_ADDR_CALC
                + generic_memory_store(node, MA_32le, addr, REG_Tu); // sw
            case 0x3000:
                CHECK_ALIGNMENT(addr, 7);
                return RISCV_LATENCY_ADDR_CALC
                + generic_memory_store(node, MA_64le, addr, REG_Tu); // sd
            }
            break;
        }

        case 0x13:
            switch (iw & 0x7000) {
            case 0x0000: // addi
                REG_D = REG_S + IMM_I;
                return RISCV_LATENCY_ARITH;
            case 0x1000: // slli
                if ((iw&0xfc000000)==0) {
                    REG_D = REG_S << IMM_SHAMT;
                    return RISCV_LATENCY_ARITH;
                } // else invalid
                break;
            case 0x2000: // slti
                REG_D = (REG_S < IMM_I) ? 1 : 0;
                return RISCV_LATENCY_ARITH;
            case 0x3000: // sltiu
                REG_D = (REG_Su < (uint64_t)IMM_I) ? 1 : 0;
                return RISCV_LATENCY_ARITH;
            case 0x4000: // xori
                REG_D = REG_S ^ IMM_I;
                return RISCV_LATENCY_ARITH;
            case 0x5000:
                if ((iw&0xfc000000)==0) { // srli
                    REG_Du = REG_Su >> IMM_SHAMT;
                    return RISCV_LATENCY_ARITH;
                } else if ((iw&0xfc000000)==0x40000000) { // srai
                    REG_D = REG_S >> IMM_SHAMT;
                    return RISCV_LATENCY_ARITH;
                } // else invalid
                break;
            case 0x6000: // ori
                REG_D = REG_S | IMM_I;
                return RISCV_LATENCY_ARITH;
            case 0x7000: // andi
                REG_D = REG_S & IMM_I;
                return RISCV_LATENCY_ARITH;
            }
            break;

        case 0x33:
            switch ((iw>>25) & 0x7f) {
            case 0x00:
                switch (iw & 0x7000) {
                case 0x0000: // add
                    REG_D = REG_S + REG_T;
                    return RISCV_LATENCY_ARITH;
                case 0x1000: // sll
                    REG_D = REG_S << (REG_T & 0x3f);
                    return RISCV_LATENCY_ARITH;
                case 0x2000: // slt
                    REG_D = (REG_S < REG_T) ? 1 : 0;
                    return RISCV_LATENCY_ARITH;
                case 0x3000: // sltu
                    REG_D = (REG_Su < REG_Tu) ? 1 : 0;
                    return RISCV_LATENCY_ARITH;
                case 0x4000: // xor
                    REG_D = REG_S ^ REG_T;
                    return RISCV_LATENCY_ARITH;
                case 0x5000: // srl
                    REG_Du = REG_Su >> (REG_T&0x3f);
                    return RISCV_LATENCY_ARITH;
                case 0x6000: // or
                    REG_D = REG_S | REG_T;
                    return RISCV_LATENCY_ARITH;
                case 0x7000: // and
                    REG_D = REG_S & REG_T;
                    return RISCV_LATENCY_ARITH;
                }
                break;

            case 0x01:
                switch (iw & 0x7000) {
                case 0x0000: // mul
                    REG_Du = REG_Su * REG_Tu;
                    return RISCV_LATENCY_MUL;
                case 0x1000: // mulh
                    REG_D = riscv_mulh(REG_S, REG_T);
                    return RISCV_LATENCY_MUL;
                case 0x2000: // mulhsu
                    REG_D = riscv_mulhsu(REG_S, REG_Tu);
                    return RISCV_LATENCY_MUL;
                case 0x3000: // mulhu
                    REG_D = riscv_mulhu(REG_Su, REG_Tu);
                    return RISCV_LATENCY_MUL;
                case 0x4000: // div
                    if ((REG_S==INT64_MIN) && (REG_T==-1))
                        REG_D = INT64_MIN;
                    else
                        REG_D = (REG_T==0) ? -1 : (REG_S / REG_T);
                    return RISCV_LATENCY_DIV64;
                case 0x5000: // divu
                    REG_Du = (REG_Tu==0) ? -1 : (REG_Su / REG_Tu);
                    return RISCV_LATENCY_DIV64;
                case 0x6000: // rem
                    if ((REG_S==INT64_MIN) && (REG_T==-1))
                        REG_D = 0;
                    else
                        REG_D = (REG_T==0) ? REG_S : (REG_S % REG_T);
                    return RISCV_LATENCY_DIV64;
                case 0x7000: // remu
                    REG_Du = (REG_Tu==0) ? REG_Su : (REG_Su % REG_Tu);
                    return RISCV_LATENCY_DIV64;
                }
                break;

            case 0x20:
                switch (iw & 0x7000) {
                case 0x0000: // sub
                    REG_D = REG_S - REG_T;
                    return RISCV_LATENCY_ARITH;
                case 0x5000: // sra
                    REG_D = REG_S >> (REG_T&0x3f);
                    return RISCV_LATENCY_ARITH;
                }
                break;
            }
            break;

        case 0x0f:
            if ((iw&0xf00fffff)==0x0000000f) { // fence
                // nop with this memory architecture
                return RISCV_LATENCY_FENCE;
            } else if (iw==0x0000100f) { // fence.i
                // nop with this memory architecture
                return RISCV_LATENCY_FENCE;
            }
            break;

        case 0x73:
            if ((iw & 0x7000)==0) {
                if (iw==0x00000073) { // ecall
                    return ic_exception(node, RISCV_EXCEPTION_ECALL);
                }
                if (iw==0x00100073) { // ebreak
                    return ic_exception(node, RISCV_EXCEPTION_EBREAK);
                }
                if (iw==0x10000073) { // eret
                    node->nextpc = node->core.riscv.mepc;
                    return RISCV_LATENCY_JALR;
                }
            } else {
                unsigned no = (iw>>20)&0xfff;
                if ((((iw&0xfff0307f)==0x78001073) || ((iw&0xfff0307f)==0x78002073))
                    && (((iw&0xc000)==0xc000) || (((iw&0x4000)==0) && ((REG_S&1)!=0))))
                {
                    node->state = CS_STOPPED;
                    return IC_STOP;
                }
                if (csr_valid(iw)) {
                    uint64_t u = csr_read(node, no);
                    switch (iw & 0x7000) {
                    case 0x1000: // csrrw
                        csr_write(node, no, REG_S);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    case 0x2000: // csrrs
                        csr_write(node, no, u | REG_S);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    case 0x3000: // csrrc
                        csr_write(node, no, u & ~REG_S);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    case 0x5000: // csrrwi
                        csr_write(node, no, IMM_Z);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    case 0x6000: // csrrsi
                        csr_write(node, no, u | IMM_Z);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    case 0x7000: // csrrci
                        csr_write(node, no, u & ~IMM_Z);
                        REG_D = u;
                        return RISCV_LATENCY_CSR;
                    }
                }
            }
            break;

        // RV64I
        case 0x1b:
            switch (iw & 0x7000) {
            case 0x0000: // addiw
                REG_D = (int32_t)(REG_S + IMM_I);
                return RISCV_LATENCY_ARITH;
            case 0x1000: // slliw
                if ((iw&0xfc000000)==0) {
                    REG_D = (int32_t)(REG_S << (IMM_SHAMT&0x1f));
                    return RISCV_LATENCY_ARITH;
                } // else invalid
                break;
            case 0x5000:
                if ((iw&0xfc000000)==0) { // srliw
                    REG_Du = (int32_t)((uint32_t)REG_Su >> (IMM_SHAMT&0x1f));
                    return RISCV_LATENCY_ARITH;
                } else if ((iw&0xfc000000)==0x40000000) { // sraiw
                    REG_D = (int32_t)((int32_t)REG_S >> (IMM_SHAMT&0x1f));
                    return RISCV_LATENCY_ARITH;
                } // else invalid
            }
            break;

        case 0x3b:
            switch ((iw>>25) & 0x7f) {
            case 0x00:
                switch (iw & 0x7000) {
                case 0x0000: // addw
                    REG_D = (int32_t)(REG_S + REG_T);
                    return RISCV_LATENCY_ARITH;
                case 0x1000: // sllw
                    REG_D = (int32_t)((int32_t)REG_S << (REG_T & 0x1f));
                    return RISCV_LATENCY_ARITH;
                case 0x5000: // srlw
                    REG_Du = (int32_t)((uint32_t)REG_Su >> (REG_T&0x1f));
                    return RISCV_LATENCY_ARITH;
                }
                break;

            case 0x01:
                switch (iw & 0x7000) {
                case 0x0000: // mulw
                    REG_D = (int32_t)(REG_S * REG_T);
                    return RISCV_LATENCY_MUL;
                case 0x4000: // divw
                {
                    int32_t a = REG_S;
                    int32_t b = REG_T;
                    REG_D = (b==0) ? -1 : 
                        ((a==INT32_MIN) && (b==-1) ? INT32_MIN :
                        (int32_t)(a/b));
                    return RISCV_LATENCY_DIV32;
                }
                case 0x5000: // divuw
                {
                    uint32_t a = REG_Su;
                    uint32_t b = REG_Tu;
                    REG_D = (b==0) ? -1 : (int32_t)(a/b);
                    return RISCV_LATENCY_DIV32;
                }
                case 0x6000: // remw
                {
                    int32_t a = REG_S;
                    int32_t b = REG_T;
                    REG_D = (b==0) ? (int32_t)a : 
                        ((a==INT32_MIN) && (b==-1) ? 0 :
                        (int32_t)(a%b));
                    return RISCV_LATENCY_DIV32;
                }
                case 0x7000: // remuw
                {
                    uint32_t a = REG_Su;
                    uint32_t b = REG_Tu;
                    REG_D = (b==0) ? (int32_t)a : (int32_t)(a%b);
                    return RISCV_LATENCY_DIV32;
                }
                }
                break;

            case 0x20:
                switch (iw & 0x7000) {
                case 0x0000: // subw
                    REG_D = (int32_t)(REG_S - REG_T);
                    return RISCV_LATENCY_ARITH;
                case 0x5000: // sraw
                    REG_D = (int32_t)((int32_t)REG_S >> (REG_T&0x1f));
                    return RISCV_LATENCY_ARITH;
                }
                break;
            }
            break;

        // floating point
        case 0x07:
        {
            addr_t addr = REG_S + IMM_I;
            
            instruction_class_t latency = RISCV_LATENCY_ADDR_CALC;
            
            // next_instruction.DREG_S = DREG_D and next_instruction uses DREG_S
            if (riscv_instruction_uses_dreg_s(next_iw) && nBRu(19, 15) == BRu(11, 7)) {
                latency = RISCV_LATENCY_INTERLOCK;
            }
            // next_instruction.DREG_T = DREG_D and next_instruction uses DREG_T
            if (riscv_instruction_uses_dreg_t(next_iw) && nBRu(24, 20) == BRu(11, 7)) {
                latency = RISCV_LATENCY_INTERLOCK;
            }
            // next_instruction.DREG_T = DREG_U and next_instruction uses DREG_U
            if (riscv_instruction_uses_dreg_u(next_iw) && nBRu(31, 27) == BRu(11, 7)) {
                latency = RISCV_LATENCY_INTERLOCK;
            }
            
            switch (iw & 0x7000) {
            case 0x2000:  // flw
            {
                CHECK_ALIGNMENT(addr, 3);
                uint64_t u;
                uint16_t lat = generic_memory_load_u64(node, MA_32le, addr, &u);
                x32.u = u;
                DREG_D = boxing_float(x32.f);
                return latency+lat;
            }
            case 0x3000:  // fld
            {
                CHECK_ALIGNMENT(addr, 7);
                uint16_t lat = generic_memory_load_u64(node, MA_64le, addr, &x64.u);
                DREG_D = x64.f;
                return latency+lat;
            }
            }
            break;
        }

        case 0x27:
        {
            addr_t addr = REG_S + IMM_S;
            
            switch (iw & 0x7000) {
            case 0x2000:  // fsw
                CHECK_ALIGNMENT(addr, 3);
                x64.f = DREG_T; // no unboxing!
                return RISCV_LATENCY_ADDR_CALC
                    + generic_memory_store(node, MA_32le, addr, x64.u);
            case 0x3000:  // fsd
                CHECK_ALIGNMENT(addr, 7);
                x64.f = DREG_T;
                return RISCV_LATENCY_ADDR_CALC
                    + generic_memory_store(node, MA_64le, addr, x64.u);
            }
            break;
        }

        case 0x43:
            switch ((iw>>25)&3) {
            case 0x00: // fmadd.s
                set_rm(node, iw);
                DREG_D = boxing_float(fmaf(SREG_S, SREG_T, SREG_U));
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            case 0x01: // fmadd.d
                set_rm(node, iw);
                DREG_D = fma(DREG_S, DREG_T, DREG_U);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            }
            break;

        case 0x47:
            switch ((iw>>25)&3) {
            case 0x00: // fmsub.s
                set_rm(node, iw);
                DREG_D = boxing_float(fmaf(SREG_S, SREG_T, -SREG_U));
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            case 0x01: // fmsub.d
                set_rm(node, iw);
                DREG_D = fma(DREG_S, DREG_T, -DREG_U);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            }
            break;

        case 0x4b:
            switch ((iw>>25)&3) {
            case 0x00: // fnmsub.s
                set_rm(node, iw);
                DREG_D = boxing_float(-fmaf(SREG_S, SREG_T, -SREG_U));
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            case 0x01: // fnmsub.d
                set_rm(node, iw);
                DREG_D = -fma(DREG_S, DREG_T, -DREG_U);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            }
            break;

        case 0x4f:
            switch ((iw>>25)&3) {
            case 0x00: // fnmadd.s
                set_rm(node, iw);
                DREG_D = boxing_float(-fmaf(SREG_S, SREG_T, SREG_U));
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            case 0x01: // fnmadd.d
                set_rm(node, iw);
                DREG_D = -fma(DREG_S, DREG_T, DREG_U);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMA;
            }
            break;


        case 0x53:
            switch ((iw>>25)&0x7f) {
            case 0x00: // fadd.s
                set_rm(node, iw);
                DREG_D = canonical_nan_float(SREG_S + SREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FADD;
            case 0x01: // fadd.d
                set_rm(node, iw);
                DREG_D = canonical_nan_double(DREG_S + DREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FADD;
            case 0x04: // fsub.s
                set_rm(node, iw);
                DREG_D = canonical_nan_float(SREG_S - SREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FADD;
            case 0x05: // fsub.d
                set_rm(node, iw);
                DREG_D = canonical_nan_double(DREG_S - DREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FADD;
            case 0x08: // fmul.s
                set_rm(node, iw);
                DREG_D = canonical_nan_float(SREG_S * SREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMUL;
            case 0x09: // fmul.d
                set_rm(node, iw);
                DREG_D = canonical_nan_double(DREG_S * DREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FMUL;
            case 0x0c: // fdiv.s
                set_rm(node, iw);
                DREG_D = canonical_nan_float(SREG_S / SREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FDIV_S;
            case 0x0d: // fdiv.d
                set_rm(node, iw);
                DREG_D = canonical_nan_double(DREG_S / DREG_T);
                update_client_fp_env(node);
                return RISCV_LATENCY_FDIV_D;
            case 0x10:
                switch (iw&0x7000) {
                case 0x0000: // fsgnj.s
                    DREG_D = boxing_float(copysignf(SREG_S, SREG_T));
                    return RISCV_LATENCY_FMV;
                case 0x1000: // fsgnjn.s
                    DREG_D = boxing_float(copysignf(SREG_S, -SREG_T));
                    return RISCV_LATENCY_FMV;
                case 0x2000: // fsgnjx.s
                    x32.f = SREG_S;
                    y32.f = SREG_T;
                    x32.i = (x32.i & 0x7fffffff) | ((x32.i^y32.i) & 0x80000000);
                    DREG_D = boxing_float(x32.f);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x11:
                switch (iw&0x7000) {
                case 0x0000: // fsgnj.d
                    DREG_D = copysign(DREG_S, DREG_T);
                    return RISCV_LATENCY_FMV;
                case 0x1000: // fsgnjn.d
                    DREG_D = copysign(DREG_S, -DREG_T);
                    return RISCV_LATENCY_FMV;
                case 0x2000: // fsgnjx.d
                    x64.f = DREG_S;
                    y64.f = DREG_T;
                    x64.u = (x64.u & 0x7fffffffffffffff) | ((x64.u^y64.u) & 0x8000000000000000);
                    DREG_D = x64.f;
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x14:
                switch (iw&0x7000) {
                case 0x0000: // fmin.s
                    x32.f = SREG_S;
                    y32.f = SREG_T;
                    if ((x32.u|y32.u) == 0x80000000) { // fmin(+0.0, -0.0) = -0.0
                        x64.u = 0xffffffff80000000;
                        DREG_D = x64.f;
                    } else {
                        DREG_D = canonical_nan_float(fminf(x32.f, y32.f));
                        if (issignaling_float(x32.f) ||
                            issignaling_float(y32.f))
                        {
                            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
                        }
                    }
                    return RISCV_LATENCY_FMIN;
                case 0x1000: // fmax.s
                    x32.f = SREG_S;
                    y32.f = SREG_T;
                    if ((x32.u|y32.u) == 0x80000000) { // fmax(+0.0, -0.0) = +0.0
                        x64.i = 0xffffffff00000000 | (x32.i&y32.i);
                        DREG_D = x64.f;
                    } else {
                        DREG_D = canonical_nan_float(fmaxf(x32.f, y32.f));
                        if (issignaling_float(x32.f) ||
                            issignaling_float(y32.f))
                        {
                            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
                        }
                    }
                    return RISCV_LATENCY_FMIN;
                }
                break;
            case 0x15:
                switch (iw&0x7000) {
                case 0x0000: // fmin.d
                    x64.f = DREG_S;
                    y64.f = DREG_T;
                    if ((x64.u|y64.u) == 0x8000000000000000) { // fmin(+0.0, -0.0) = -0.0
                        DREG_D = -0.0;
                    } else {
                        DREG_D = canonical_nan_double(fmin(x64.f, y64.f));
                        if (issignaling_double(x64.f) ||
                            issignaling_double(y64.f))
                        {
                            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
                        }
                    }
                    return RISCV_LATENCY_FMIN;
                case 0x1000: // fmax.d
                    x64.f = DREG_S;
                    y64.f = DREG_T;
                    if ((x64.u|y64.u) == 0x8000000000000000) { // fmax(+0.0, -0.0) = +0.0
                        x64.u = x64.u&y64.u;
                        DREG_D = x64.f;
                    } else {
                        DREG_D = canonical_nan_double(fmax(x64.f, y64.f));
                        if (issignaling_double(x64.f) ||
                            issignaling_double(y64.f))
                        {
                            node->core.riscv.fcsr |= RISCV_FFLAGS_NV;
                        }
                    }
                    return RISCV_LATENCY_FMIN;
                }
                break;


            case 0x20: 
                if ((iw&0x01f00000)==0x00100000) { // fcvt.s.d
                    set_rm(node, iw);
                    DREG_D = canonical_nan_float((float)DREG_S);
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x21: 
                if ((iw&0x01f00000)==0x00000000) { // fcvt.d.s
                    set_rm(node, iw);
                    DREG_D = canonical_nan_double((double)SREG_S);
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x2c: 
                if ((iw&0x01f00000)==0) { // fsqrt.s
                    set_rm(node, iw);
                    DREG_D = canonical_nan_float(sqrtf(SREG_S));
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FSQRT_S;
                }
                break;
            case 0x2d: 
                if ((iw&0x01f00000)==0) { // fsqrt.d
                    set_rm(node, iw);
                    DREG_D = canonical_nan_double(sqrt(DREG_S));
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FSQRT_D;
                }
                break;
            case 0x50:
                switch (iw&0x7000) {
                case 0x0000: // fle.s
                    REG_D = (SREG_S <= SREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                case 0x1000: // flt.s
                    REG_D = (SREG_S < SREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                case 0x2000: // feq.s
                    REG_D = (SREG_S == SREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                }
                break;
            case 0x51:
                switch (iw&0x7000) {
                case 0x0000: // fle.d
                    REG_D = (DREG_S <= DREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                case 0x1000: // flt.d
                    REG_D = (DREG_S < DREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                case 0x2000: // feq.d
                    REG_D = (DREG_S == DREG_T) ? 1 : 0;
                    update_client_fp_env(node);
                    return RISCV_LATENCY_FCMP;
                }
                break;
            // comment on the conversion to integer:
            // Spike uses soft-float for the emulation of floating point 
            // arithmetics. Hence it resembles soft-float's behaviour on
            // returning values when the invalid exception occurs. For
            // conversion to integers this means:
            // integers with sign: return the smallest/largest possible
            // value if underflow/overflow
            // integers without sign: return largest possible value (-1)

            case 0x60:
                switch ((iw>>20)&0x1f) {
                case 0x00: // fcvt.w.s
                    set_rm(node, iw);
                    REG_D = (int32_t)riscv_fcvt(node, SREG_S, INT32_MIN, INT32_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x01: // fcvt.wu.s
                    set_rm(node, iw);
                    REG_D = (int32_t)riscv_fcvt(node, SREG_S, 0, UINT32_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x02: // fcvt.l.s
                    set_rm(node, iw);
                    REG_D = riscv_fcvt(node, SREG_S, INT64_MIN, INT64_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x03: // fcvt.lu.s
                    set_rm(node, iw);
                    REG_D = riscv_fcvt(node, SREG_S, 0, UINT64_MAX);
                    return RISCV_LATENCY_F2I;
                }
                break;
            case 0x61:
                switch ((iw>>20)&0x1f) {
                case 0x00: // fcvt.w.d
                    set_rm(node, iw);
                    REG_D = (int32_t)riscv_fcvt(node, DREG_S, INT32_MIN, INT32_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x01: // fcvt.wu.d
                    set_rm(node, iw);
                    REG_D = (int32_t)riscv_fcvt(node, DREG_S, 0, UINT32_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x02: // fcvt.l.d
                    set_rm(node, iw);
                    REG_D = riscv_fcvt(node, DREG_S, INT64_MIN, INT64_MAX);
                    return RISCV_LATENCY_F2I;
                case 0x03: // fcvt.lu.d
                    set_rm(node, iw);
                    REG_D = riscv_fcvt(node, DREG_S, 0, UINT64_MAX);
                    return RISCV_LATENCY_F2I;
                }
                break;

            case 0x68:
                switch ((iw>>20)&0x1f) {
                case 0x00: // fcvt.s.w
                    DREG_D = boxing_float((float)WREG_S);
                    return RISCV_LATENCY_I2F;
                case 0x01: // fcvt.s.wu
                    DREG_D = boxing_float((float)WREG_Su);
                    return RISCV_LATENCY_I2F;
                case 0x02: // fcvt.s.l
                    DREG_D = boxing_float((float)REG_S);
                    return RISCV_LATENCY_I2F;
                case 0x03: // fcvt.s.lu
                    DREG_D = boxing_float((float)REG_Su);
                    return RISCV_LATENCY_I2F;
                }
                break;
            case 0x69:
                switch ((iw>>20)&0x1f) {
                case 0x00: // fcvt.d.w
                    DREG_D = (double)WREG_S;
                    return RISCV_LATENCY_I2F;
                case 0x01: // fcvt.d.wu
                    DREG_D = (double)WREG_Su;
                    return RISCV_LATENCY_I2F;
                case 0x02: // fcvt.d.l
                    DREG_D = (double)REG_S;
                    return RISCV_LATENCY_I2F;
                case 0x03: // fcvt.d.lu
                    DREG_D = (double)REG_Su;
                    return RISCV_LATENCY_I2F;
                }
                break;
            case 0x70:
                if ((iw&0x01f07000)==0x0000) { // fmv.x.w
                    x64.f = DREG_S;
                    REG_D = (int32_t)x64.u;
                    return RISCV_LATENCY_FMV;
                } else if ((iw&0x01f07000)==0x1000) { // fclass.s
                    REG_D = fclass_float(SREG_S);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x71:
                if ((iw&0x01f07000)==0) { // fmv.x.d
                    x64.f = DREG_S;
                    REG_D = x64.u;
                    return RISCV_LATENCY_FMV;
                } else if ((iw&0x01f07000)==0x1000) { // fclass.d
                    REG_D = fclass_double(DREG_S);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x78:
                if ((iw&0x01f07000)==0) { // fmv.w.x
                    x32.i = REG_S;
                    DREG_D = boxing_float(x32.f);
                    return RISCV_LATENCY_FMV;
                }
                break;
            case 0x79:
                if ((iw&0x01f07000)==0) { // fmv.d.x
                    x64.u = REG_S;
                    DREG_D = x64.f;
                    return RISCV_LATENCY_FMV;
                }
                break;
            }
            break;

        // RC/MC
        case 0x6b:
        {
            switch (iw & 0x7000) {
                case 0x0000: // send s t
                {
                    if (node->noc_send_flit(node, REG_S, REG_T)) {
                        return 1;
                    }
                    node->state = CS_SEND_BLOCKED;
                    return IC_BLOCKED;
                }
                case 0x1000: // cong d
                {
                    instruction_class_t additional_cycles = 0;
                    // next_instruction.REG_S = REG_D and next_instruction uses REG_S
                    if (riscv_instruction_uses_reg_s(next_iw) && nBRu(19, 15) == BRu(11, 7)) {
                        additional_cycles = 1;
                    }
                    // next_instruction.REG_T = REG_D and next_instruction uses REG_T
                    if (riscv_instruction_uses_reg_t(next_iw) && nBRu(24, 20) == BRu(11, 7)) {
                        additional_cycles = 1;
                    }
                    REG_D = !node->noc_sender_ready(node);
                    return 1 + additional_cycles;
                }
                case 0x4000: // recv d s
                case 0x5000: // probe d s
                case 0x7000: // any d
                {
                    instruction_class_t additional_cycles = 0;
                    // Wait at least one cycle.
                    if (node->state != CS_FGMP_REQUEST) {
                        node->state = CS_FGMP_REQUEST;
                        return IC_BLOCKED;
                    }
                    // next_instruction.REG_S = REG_D and next_instruction uses REG_S
                    if (riscv_instruction_uses_reg_s(next_iw) && nBRu(19, 15) == BRu(11, 7)) {
                        additional_cycles = 1;
                    }
                    // next_instruction.REG_T = REG_D and next_instruction uses REG_T
                    if (riscv_instruction_uses_reg_t(next_iw) && nBRu(24, 20) == BRu(11, 7)) {
                        additional_cycles = 1;
                    }
                    switch (iw & 0x7000)
                    {
                        case 0x4000: // recv d s
                        {
                            flit_t flit;
                            if (node->noc_recv_flit(node, REG_S, &flit)) {
                                REG_D = flit;
                            } else {
                                node->state = CS_RECV_BLOCKED;
                                return IC_BLOCKED;
                            }
                            break;
                        }
                        case 0x5000: // probe d s
                            REG_D = node->noc_probe_rank(node, REG_S) ? 1 : 0;
                            break;
                        case 0x7000: // any d
                            REG_D = node->noc_probe_any(node);
                            break;

                    }
                    return 1 + additional_cycles;
                }
                /*case 0x6000: // wait d - unsupported
                {
                    rank_t rank = node->noc_probe_any(node);
                    if (rank<MAX_RANK) {
                        REG_D = rank;
                        return 1;
                    }
                    node->state = CS_RECV_BLOCKED;
                    return IC_BLOCKED;
                }*/
            }
        }

        // PIMP
        case 0x5b:
        {
            switch (iw & 0x7000) {
                case 0x0000: // snd s t
                    if (!node->noc_send_flit(node, REG_S, REG_T)) {
                        fatal("RISC-V exception: send buffer full");
                    }
                    return 1;
                case 0x2000: // rcvn d
                    REG_D = node->noc_probe_any(node); 
                        // UGLY, but working
                    return 1;
                case 0x3000: // rcvp d
                {
                    flit_t flit;
                    if (!node->noc_recv_flit(node, node->noc_probe_any(node), &flit)) {
                        fatal("RISC-V exception: recv buffer empty");
                    }
                    REG_D = flit;
                    return 1;
                }
            }
        }

        // PIMP branches
        case 0x7b:
        {
            addr_t disp = IMM_B;
            switch (iw & 0x7000) {
            case 0x0000: return ic_branch(node, !node->noc_sender_ready(node), disp); // bsf
            case 0x1000: return ic_branch(node, node->noc_sender_ready(node), disp); // bsnf
            case 0x2000: return ic_branch(node, node->noc_probe_any(node)<0, disp); // bre
            case 0x3000: return ic_branch(node, node->noc_probe_any(node)>=0, disp); // brne
            }
            break;
        }
    }

    // alternative: 
    // return ic_exception(node, RISCV_EXCEPTION_ILLEGAL_INSTRUCTION);
    node->state = CS_UNKNOWN_INSTRUCTION;
    return IC_STOP;
}


// Simulate one cycle
instruction_class_t riscv_one_cycle(node_t *node)
{
    uint32_t iw;
    uint32_t next_iw;
    uint_fast32_t pc = node->pc;

    memory_fetch_32le(node, pc, &iw);
    node->nextpc = pc + 4;
    memory_fetch_32le(node, node->nextpc, &next_iw);
    node->retired++;
    return riscv_execute_iw(node, iw, next_iw);
}


// push the command line arguments on the stack
// argbuf is the concatination of argc zero-terminated strings
void riscv_set_argv(node_t *node, int argc, char *argbuf)
{
    uint_fast64_t i, len=0;

    for (i=0; i<argc; i++) {
        while (argbuf[len]!=0) len++;
        len++;
    }

    addr_t sp = (RISCV_STACK_END - 8*(argc+2) - len) & RISCV_STACK_ALIGN_MASK;
    addr_t argbuf_addr = sp+8*(argc+2);
    addr_t ofs = 0;

    generic_memory_store(node, MA_64le, sp, argc);
    for (i=0; i<argc; i++) {
        generic_memory_store(node, MA_64le, sp+8+8*i, argbuf_addr+ofs);
        while (argbuf[ofs]!=0) ofs++;
        ofs++;
    }
    generic_memory_store(node, MA_64le, sp+8+8*argc, 0); // argv[argc] = 0
    memory_write(node, argbuf_addr, (uint8_t *)argbuf, len);
    node->core.riscv.reg[2] = sp;
}


// Init context
void riscv_init_context(node_t *node)
{
    uint_fast32_t i;

    node->set_argv = riscv_set_argv;

    memory_init(node, MT_PAGED_32BIT, 0x100000000LL);

    node->cycle = 0;
    node->retired = 0;
    node->state = CS_RUNNING;
    node->pc = 0;

    node->core_type = CT_riscv;
    node->one_cycle = &riscv_one_cycle;

    node->core.riscv.mstatus = 0;
    node->core.riscv.mtvec = 0;
    node->core.riscv.mscratch = 0;
    node->core.riscv.mepc = 0; // undef
    node->core.riscv.mcause = 0; // undef
    node->core.riscv.fcsr = 0;

    for (i = 0; i < 31; i++)
        node->core.riscv.reg[i] = 0;
}


// Remove context from memory and free memory blocks
void riscv_finish_context(node_t *node)
{
    memory_finish(node);
}







static const char *reg_names[32] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};


static char *cat_str(char *d, const char *s)
{
    strcpy(d, s);
    return d + strlen(s);
}


static char *cat_reg(char *d, unsigned reg)
{
    return cat_str(d, reg_names[reg]);
}


static char *cat_freg(char *d, unsigned reg)
{
    return d + sprintf(d, "f%d", reg);
}



static char *cat_csr(char *d, unsigned csr)
{
    switch (csr) {
        case RISCV_CSR_FFLAGS:  return cat_str(d, "fflags");
        case RISCV_CSR_FRM:     return cat_str(d, "frm");
        case RISCV_CSR_FCSR:    return cat_str(d, "fcsr");
        case RISCV_CSR_CYCLE:   return cat_str(d, "cycle");
        case RISCV_CSR_TIME:    return cat_str(d, "time");
        case RISCV_CSR_INSTRET: return cat_str(d, "instret");
        case RISCV_CSR_CYCLEH:  return cat_str(d, "cycleh");
        case RISCV_CSR_TIMEH:   return cat_str(d, "timeh");
        case RISCV_CSR_INSTRETH:return cat_str(d, "instreth");
        case RISCV_CSR_MCPUID:  return cat_str(d, "mcpuid");
        case RISCV_CSR_MIMPID:  return cat_str(d, "mimpid");
        case RISCV_CSR_MHARTID: return cat_str(d, "mhartid");
        case RISCV_CSR_MSTATUS: return cat_str(d, "mstatus");
        case RISCV_CSR_MTVEC:   return cat_str(d, "mtvec");
        case RISCV_CSR_MTDELEG: return cat_str(d, "mtdeleg");
        case RISCV_CSR_MIE:     return cat_str(d, "mie");
        case RISCV_CSR_MTIMECMP:return cat_str(d, "mtimecmp");
        case RISCV_CSR_MTIME:   return cat_str(d, "mtime");
        case RISCV_CSR_MTIMEH:  return cat_str(d, "mtimeh");
        case RISCV_CSR_MSCRATCH:return cat_str(d, "mscratch");
        case RISCV_CSR_MEPC:    return cat_str(d, "mepc");
        case RISCV_CSR_MCAUSE:  return cat_str(d, "mcause");
 
        case RISCV_CSR_MTOHOST: return cat_str(d, "mtohost");
        case RISCV_CSR_MFROMHOST:return cat_str(d, "mfromhost");
        case RISCV_CSR_PUTCHAR: return cat_str(d, "putchar");
        case RISCV_CSR_MAXCID:  return cat_str(d, "maxcid");
        case RISCV_CSR_CID:     return cat_str(d, "cid");
        case RISCV_CSR_NOCDIM:  return cat_str(d, "nocdim");
        case RISCV_CSR_SENDRDY: return cat_str(d, "sendrdy");
        case RISCV_CSR_NEXTRECV:return cat_str(d, "nextrecv");
    }
    return d + sprintf(d, "0x%03x", csr);
}


static char *cat_uint(char *d, unsigned long imm)
{
    return d + sprintf(d, "+%lu", imm);
}


static char *cat_int(char *d, long imm)
{
    return d + sprintf(d, "%+ld", imm);
}


static char *cat_addr(char *d, unsigned long a)
{
    return d + sprintf(d, "%06lx", a);
}


static const char *rounding_mode[8] = {
    "rne", "rtz", "rdn", "rup", "rmm", "101", "110", "dyn"};


static void disasm(char *d, const char *f, addr_t pc, uint_fast32_t iw)
{
    char c;
    do {
        c = *f++;
        if (c=='%') {
            c = *f++;
            switch (c) {
            case 'b': d = cat_addr(d, pc+IMM_B); break;
            case 'c': d = cat_csr(d, (iw>>20)&0xfff); break;
            case 'd': d = cat_reg(d, BRu(11, 7)); break;
            case 'D': d = cat_freg(d, BRu(11, 7)); break;
            case 'h': d = cat_uint(d, IMM_SHAMT); break;
            case 'i': d = cat_int(d, IMM_I); break;
            case 'j': d = cat_addr(d, pc+IMM_J); break;
            case 'k': d = cat_int(d, IMM_S); break;
            case 'r': d = cat_str(d, rounding_mode[BRu(14, 12)]); break;
            case 's': d = cat_reg(d, BRu(19, 15)); break;
            case 'S': d = cat_freg(d, BRu(19, 15)); break;
            case 't': d = cat_reg(d, BRu(24, 20)); break;
            case 'T': d = cat_freg(d, BRu(24, 20)); break;
            case 'u': d = d + sprintf(d, "0x%05"PRIx64, IMM_U>>12); break;
            case 'U': d = cat_freg(d, BRu(31, 27)); break;
            case 'z': d = cat_uint(d, IMM_Z); break;
            default: *d++ = c;
            }
        }
        else *d++ = c;
    }
    while (c!=0);
}


#define D2(format)	disasm(d, format, pc, iw); return 2
#define D4(format)	disasm(d, format, pc, iw); return 4


int riscv_disasm_iw(char *d, addr_t pc, uint_fast32_t iw)
{
    switch (iw & 0x7f) {
    case 0x37: D4("lui\t%d, %u");
    case 0x17: D4("auipc\t%d, %u");
    case 0x6f: 
        switch (iw & 0x0f80) { 
        case 0x0000:    D4("j\t%j"); // d=zero
        case 0x0080:    D4("jal\t%j"); // d=ra
        default:        D4("jal\t%d, %j");
        }
    case 0x67: if ((iw & 0x7000)==0) {
            switch (iw & 0x0f80) {
            case 0x0000:    if (iw==0x8067) {
                                D4("ret");
                            } else if ((iw&0xfff00000)==0) {
                                D4("jr\t%s");
                            } else {
                                D4("jr\t%s, %i");
                            }
            case 0x0080:    if ((iw&0xfff00000)==0) {
                                D4("jalr\t%s");
                            } else {
                                D4("jalr\t%s, %i");
                            }
            default:        D4("jalr\t%d, %s, %i");
            }
        }
        break;
    case 0x63: 
        switch (iw & 0x7000) {
        case 0x0000: D4("beq\t%s, %t, %b");
        case 0x1000: D4("bne\t%s, %t, %b");
        case 0x4000: D4("blt\t%s, %t, %b");
        case 0x5000: D4("bge\t%s, %t, %b");
        case 0x6000: D4("bltu\t%s, %t, %b");
        case 0x7000: D4("bgeu\t%s, %t, %b");
        }
        break;

    case 0x03:
        switch (iw & 0x7000) {
        case 0x0000: D4("lb\t%d, %i(%s)");
        case 0x1000: D4("lh\t%d, %i(%s)");
        case 0x2000: D4("lw\t%d, %i(%s)");
        case 0x3000: D4("ld\t%d, %i(%s)");
        case 0x4000: D4("lbu\t%d, %i(%s)");
        case 0x5000: D4("lhu\t%d, %i(%s)");
        case 0x6000: D4("lwu\t%d, %i(%s)");
        }
        break;

    case 0x23:
        switch (iw & 0x7000) {
        case 0x0000: D4("sb\t%t, %k(%s)");
        case 0x1000: D4("sh\t%t, %k(%s)");
        case 0x2000: D4("sw\t%t, %k(%s)");
        case 0x3000: D4("sd\t%t, %k(%s)");
        }
        break;

    case 0x13:
        switch (iw & 0x7000) {
        case 0x0000: D4("add\t%d, %s, %i");
        case 0x1000: if ((iw&0xfc000000)==0) {
                        D4("sll\t%d, %s, %i");
                     }
                     break;
        case 0x2000: D4("slt\t%d, %s, %i");
        case 0x3000: D4("sltu\t%d, %s, %i");
        case 0x4000: D4("xor\t%d, %s, %i");
        case 0x5000: if ((iw&0xfc000000)==0) { 
                        D4("srl\t%d, %s, %i"); 
                     } else if ((iw&0xfc000000)==0x40000000) {
                        D4("sra\t%d, %s, %i"); 
                     }
                     break;
        case 0x6000: D4("or\t%d, %s, %i");
        case 0x7000: D4("and\t%d, %s, %i");
        }
        break;

    case 0x33:
        switch ((iw>>25) & 0x7f) {
        case 0x00:
            switch (iw & 0x7000) {
            case 0x0000: D4("add\t%d, %s, %t");
            case 0x1000: D4("sll\t%d, %s, %t");
            case 0x2000: D4("slt\t%d, %s, %t");
            case 0x3000: D4("sltu\t%d, %s, %t");
            case 0x4000: D4("xor\t%d, %s, %t");
            case 0x5000: D4("srl\t%d, %s, %t"); 
            case 0x6000: D4("or\t%d, %s, %t");
            case 0x7000: D4("and\t%d, %s, %t");
            }
            break;
        case 0x01:
            switch (iw & 0x7000) {
            case 0x0000: D4("mul\t%d, %s, %t");
            case 0x1000: D4("mulh\t%d, %s, %t");
            case 0x2000: D4("mulhsu\t%d, %s, %t");
            case 0x3000: D4("mulhu\t%d, %s, %t");
            case 0x4000: D4("div\t%d, %s, %t");
            case 0x5000: D4("divu\t%d, %s, %t"); 
            case 0x6000: D4("rem\t%d, %s, %t");
            case 0x7000: D4("remu\t%d, %s, %t");
            }
            break;
        case 0x20:
            switch (iw & 0x7000) {
            case 0x0000: D4("sub\t%d, %s, %t");
            case 0x5000: D4("sra\t%d, %s, %t"); 
            }
            break;
        }
        break;

    case 0x0f:
        if ((iw&0xf00fffff)==0x0000000f) {
            D4("fence");
        } else if (iw==0x0000100f) {
            D4("fence.i");
        }
        break;

    case 0x73:
        if ((iw & 0x7000)==0) {
            if (iw==0x00000073) {
                D4("ecall");
            } else if (iw==0x00100073) { 
                D4("ebreak");
            }
        } else {
            switch (iw & 0x7000) {
            case 0x1000: D4("csrrw\t%d, %c, %s");
            case 0x2000: D4("csrrs\t%d, %c, %s");
            case 0x3000: D4("csrrc\t%d, %c, %s");
            case 0x5000: D4("csrrwi\t%d, %c, %z");
            case 0x6000: D4("csrrsi\t%d, %c, %z");
            case 0x7000: D4("csrrci\t%d, %c, %z");
            }
        }
        break;

    case 0x1b:
        switch (iw & 0x7000) {
        case 0x0000: D4("addw\t%d, %s, %i");
        case 0x1000: if ((iw&0xfc000000)==0) {
                        D4("sllw\t%d, %s, %i");
                     }
                     break;
        case 0x5000: if ((iw&0xfc000000)==0) { 
                        D4("srlw\t%d, %s, %i"); 
                     } else if ((iw&0xfc000000)==0x40000000) {
                        D4("sraw\t%d, %s, %i"); 
                     }
        }
        break;

    case 0x3b:
        switch ((iw>>25) & 0x7f) {
        case 0x00:
            switch (iw & 0x7000) {
            case 0x0000: D4("addw\t%d, %s, %t");
            case 0x1000: D4("sllw\t%d, %s, %t");
            case 0x5000: D4("srlw\t%d, %s, %t"); 
            }
            break;
        case 0x01:
            switch (iw & 0x7000) {
            case 0x0000: D4("mulw\t%d, %s, %t");
            case 0x4000: D4("divw\t%d, %s, %t");
            case 0x5000: D4("divuw\t%d, %s, %t"); 
            case 0x6000: D4("remw\t%d, %s, %t");
            case 0x7000: D4("remuw\t%d, %s, %t");
            }
            break;
        case 0x20:
            switch (iw & 0x7000) {
            case 0x0000: D4("subw\t%d, %s, %t");
            case 0x5000: D4("sraw\t%d, %s, %t"); 
            }
            break;
        }
        break;

    case 0x07:
        switch (iw & 0x7000) {
        case 0x2000:  D4("flw\t%D, %i(%s)");
        case 0x3000:  D4("fld\t%D, %i(%s)");
        }
        break;

    case 0x27:
        switch (iw & 0x7000) {
        case 0x2000:  D4("fsw\t%T, %k(%s)");
        case 0x3000:  D4("fsd\t%T, %k(%s)");
        }
        break;

    case 0x43:
        switch ((iw>>25)&3) {
        case 0x00: D4("fmadd.s\t%D, %S, %T, %U, %r");
        case 0x01: D4("fmadd.d\t%D, %S, %T, %U, %r");
        }
        break;

    case 0x47:
        switch ((iw>>25)&3) {
        case 0x00: D4("fmsub.s\t%D, %S, %T, %U, %r");
        case 0x01: D4("fmsub.d\t%D, %S, %T, %U, %r");
        }
        break;

    case 0x4b:
        switch ((iw>>25)&3) {
        case 0x00: D4("fnmsub.s\t%D, %S, %T, %U, %r");
        case 0x01: D4("fnmsub.d\t%D, %S, %T, %U, %r");
        }
        break;

    case 0x4f:
        switch ((iw>>25)&3) {
        case 0x00: D4("fnmadd.s\t%D, %S, %T, %U, %r");
        case 0x01: D4("fnmadd.d\t%D, %S, %T, %U, %r");
        }
        break;

    case 0x53:
        switch ((iw>>25)&0x7f) {
        case 0x00: D4("fadd.s\t%D, %S, %T, %r");
        case 0x01: D4("fadd.d\t%D, %S, %T, %r");
        case 0x04: D4("fsub.s\t%D, %S, %T, %r");
        case 0x05: D4("fsub.d\t%D, %S, %T, %r");
        case 0x08: D4("fmul.s\t%D, %S, %T, %r");
        case 0x09: D4("fmul.d\t%D, %S, %T, %r");
        case 0x0c: D4("fdiv.s\t%D, %S, %T, %r");
        case 0x0d: D4("fdiv.d\t%D, %S, %T, %r");
        case 0x10:
            switch (iw&0x7000) {
            case 0x0000: D4("fsgnj.s\t%D, %S, %T");
            case 0x1000: D4("fsgnjn.s\t%D, %S, %T");
            case 0x2000: D4("fsgnjx.s\t%D, %S, %T");
            }
            break;
        case 0x11:
            switch (iw&0x7000) {
            case 0x0000: D4("fsgnj.d\t%D, %S, %T");
            case 0x1000: D4("fsgnjn.d\t%D, %S, %T");
            case 0x2000: D4("fsgnjx.d\t%D, %S, %T");
            }
            break;
        case 0x14:
            switch (iw&0x7000) {
            case 0x0000: D4("fmin.s\t%D, %S, %T");
            case 0x1000: D4("fmax.s\t%D, %S, %T");
            }
            break;
        case 0x15:
            switch (iw&0x7000) {
            case 0x0000: D4("fmin.d\t%D, %S, %T");
            case 0x1000: D4("fmax.d\t%D, %S, %T");
            }
            break;
        case 0x20: 
            if ((iw&0x01f00000)==0x00100000) { D4("fcvt.s.d\t%D, %S"); }
            break;
        case 0x21: 
            if ((iw&0x01f00000)==0x00000000) { D4("fcvt.d.s\t%D, %S"); }
        case 0x2c: 
            if ((iw&0x01f00000)==0) { D4("fsqrt.s\t%D, %S"); }
            break;
        case 0x2d: 
            if ((iw&0x01f00000)==0) { D4("fsqrt.d\t%D, %S"); }
            break;
        case 0x50:
            switch (iw&0x7000) {
            case 0x0000: D4("fle.s\t%d, %S, %T");
            case 0x1000: D4("flt.s\t%d, %S, %T");
            case 0x2000: D4("feq.s\t%d, %S, %T");
            }
            break;
        case 0x51:
            switch (iw&0x7000) {
            case 0x0000: D4("fle.d\t%d, %S, %T");
            case 0x1000: D4("flt.d\t%d, %S, %T");
            case 0x2000: D4("feq.d\t%d, %S, %T");
            }
            break;
        case 0x60:
            switch ((iw>>20)&0x1f) {
            case 0x00: D4("fcvt.w.s\t%d, %S, %r");
            case 0x01: D4("fcvt.wu.s\t%d, %S, %r");
            case 0x02: D4("fcvt.l.s\t%d, %S, %r");
            case 0x03: D4("fcvt.lu.s\t%d, %S, %r");
            }
            break;
        case 0x61:
            switch ((iw>>20)&0x1f) {
            case 0x00: D4("fcvt.w.d\t%d, %S, %r");
            case 0x01: D4("fcvt.wu.d\t%d, %S, %r");
            case 0x02: D4("fcvt.l.d\t%d, %S, %r");
            case 0x03: D4("fcvt.lu.d\t%d, %S, %r");
            }
            break;
        case 0x68:
            switch ((iw>>20)&0x1f) {
            case 0x00: D4("fcvt.s.w\t%D, %s, %r");
            case 0x01: D4("fcvt.s.wu\t%D, %s, %r");
            case 0x02: D4("fcvt.s.l\t%D, %s, %r");
            case 0x03: D4("fcvt.s.lu\t%D, %s, %r");
            }
            break;
        case 0x69:
            switch ((iw>>20)&0x1f) {
            case 0x00: D4("fcvt.d.w\t%D, %s, %r");
            case 0x01: D4("fcvt.d.wu\t%D, %s, %r");
            case 0x02: D4("fcvt.d.l\t%D, %s, %r");
            case 0x03: D4("fcvt.d.lu\t%D, %s, %r");
            }
            break;
        case 0x70:
            if ((iw&0x01f07000)==0x0000) { D4("fmv.x.s\t%d, %S"); }
            else if ((iw&0x01f07000)==0x1000) { D4("fclass.s\t%d, %S"); }
            break;
        case 0x71:
            if ((iw&0x01f07000)==0x0000) { D4("fmv.x.d\t%d, %S"); }
            else if ((iw&0x01f07000)==0x1000) { D4("fclass.d\t%d, %S"); }
            break;
        case 0x78:
            if ((iw&0x01f07000)==0x0000) { D4("fmv.s.x\t%D, %s"); }
            break;
        case 0x79:
            if ((iw&0x01f07000)==0x0000) { D4("fmv.d.x\t%D, %s"); }
            break;
        }
        break;

    case 0x6b:
        switch (iw & 0x7000) {
        case 0x0000: D4("send\t%s, %t");
        case 0x1000: D4("cong\t%d");
        case 0x2000: D4("invmpb");
        case 0x4000: D4("recv\t%d, %s");
        case 0x5000: D4("probe\t%d, %s");
        case 0x6000: D4("wait\t%d");
        case 0x7000: D4("any\t%d");
        }
        break;

    case 0x5b:
        switch (iw & 0x7000) {
        case 0x0000: D4("snd\t%s, %t");
        case 0x2000: D4("rcvn\t%d");
        case 0x3000: D4("rcvp\t%d");
        }
        break;

    case 0x7b:
        switch (iw & 0x7000) {
        case 0x0000: D4("bsf\t%b");
        case 0x1000: D4("bsnf\t%b");
        case 0x2000: D4("bre\t%b");
        case 0x3000: D4("brne\t%b");
        }
        break;
    }



    if ((iw&3)==3) {
        sprintf(d, ".word\t0x%08"PRIxFAST32, iw);
        return 4;
    }
    sprintf(d, ".half\t0x%04"PRIxFAST32, iw);
    return 2;
}


// Disassemble an address and return the lengh of the instruction
int riscv_disasm(node_t *node, addr_t pc, char *dstr)
{
    uint32_t iw;
    memory_fetch_32le(node, pc, &iw);
    return riscv_disasm_iw(dstr, pc, iw);
}


// Print a register dump
void riscv_print_context(node_t *node)
{
    char dstr[128];
    uint32_t iw;
    memory_fetch_32le(node, node->pc, &iw);
    riscv_disasm_iw(dstr, node->pc, iw);

    user_printf("cycle %ld pc=%08x\t%s\n"
                "fcsr=%14lx ra=%16lx sp=%16lx gp=%16lx\n"
                "tp=%16lx t0=%16lx t1=%16lx t2=%16lx\n"
                "s0=%16lx s1=%16lx a0=%16lx a1=%16lx\n"
                "a2=%16lx a3=%16lx a4=%16lx a5=%16lx\n"
                "a6=%16lx a7=%16lx s2=%16lx s3=%16lx\n"
                "s4=%16lx s5=%16lx s6=%16lx s7=%16lx\n"
                "s8=%16lx s9=%16lxs10=%16lxs11=%16lx\n"
                "t3=%16lx t4=%16lx t5=%16lx t6=%16lx\n",
                node->cycle,
                node->pc, dstr,
                node->core.riscv.fcsr, node->core.riscv.reg[1],
                node->core.riscv.reg[2], node->core.riscv.reg[3],
                node->core.riscv.reg[4], node->core.riscv.reg[5],
                node->core.riscv.reg[6], node->core.riscv.reg[7],
                node->core.riscv.reg[8], node->core.riscv.reg[9],
                node->core.riscv.reg[10], node->core.riscv.reg[11],
                node->core.riscv.reg[12], node->core.riscv.reg[13],
                node->core.riscv.reg[14], node->core.riscv.reg[15],
                node->core.riscv.reg[16], node->core.riscv.reg[17],
                node->core.riscv.reg[18], node->core.riscv.reg[19],
                node->core.riscv.reg[20], node->core.riscv.reg[21],
                node->core.riscv.reg[22], node->core.riscv.reg[23],
                node->core.riscv.reg[24], node->core.riscv.reg[25],
                node->core.riscv.reg[26], node->core.riscv.reg[27],
                node->core.riscv.reg[28], node->core.riscv.reg[29],
                node->core.riscv.reg[30], node->core.riscv.reg[31]);

    user_printf("fcsr=%14lx\n"
                "f0=%16lx (%g) f1=%16lx (%g)\n"
                "f2=%16lx (%g) f3=%16lx (%g)\n"
                "f4=%16lx (%g) f5=%16lx (%g)\n"
                "f6=%16lx (%g) f7=%16lx (%g)\n", 
                node->core.riscv.fcsr,
                *(uint64_t *)(&node->core.riscv.freg[0]), node->core.riscv.freg[0],
                *(uint64_t *)(&node->core.riscv.freg[1]), node->core.riscv.freg[1],
                *(uint64_t *)(&node->core.riscv.freg[2]), node->core.riscv.freg[2],
                *(uint64_t *)(&node->core.riscv.freg[3]), node->core.riscv.freg[3],
                *(uint64_t *)(&node->core.riscv.freg[4]), node->core.riscv.freg[4],
                *(uint64_t *)(&node->core.riscv.freg[5]), node->core.riscv.freg[5],
                *(uint64_t *)(&node->core.riscv.freg[6]), node->core.riscv.freg[6],
                *(uint64_t *)(&node->core.riscv.freg[7]), node->core.riscv.freg[7]
                );

/*
    user_printf("fcsr=%14lx\n"
                "f0=%16lx (%g) f1=%16lx (%g)\n"
                "f2=%16lx (%g) f3=%16lx (%g)\n"
                "f4=%16lx (%g) f5=%16lx (%g)\n"
                "f6=%16lx (%g) f7=%16lx (%g)\n", 
                node->core.riscv.fcsr,
                *(uint64_t *)(&node->core.riscv.freg[0]), *(float *)(&node->core.riscv.freg[0]),
                *(uint64_t *)(&node->core.riscv.freg[1]), *(float *)(&node->core.riscv.freg[1]),
                *(uint64_t *)(&node->core.riscv.freg[2]), *(float *)(&node->core.riscv.freg[2]),
                *(uint64_t *)(&node->core.riscv.freg[3]), *(float *)(&node->core.riscv.freg[3]),
                *(uint64_t *)(&node->core.riscv.freg[4]), *(float *)(&node->core.riscv.freg[4]),
                *(uint64_t *)(&node->core.riscv.freg[5]), *(float *)(&node->core.riscv.freg[5]),
                *(uint64_t *)(&node->core.riscv.freg[6]), *(float *)(&node->core.riscv.freg[6]),
                *(uint64_t *)(&node->core.riscv.freg[7]), *(float *)(&node->core.riscv.freg[7])
                );
*/
}

void riscv_dump_context(const char *file, node_t *node) {
  node->core.riscv.reg[0] = 0;

  FILE *out = fopen(file, "w");
  if (!out) {
    fatal("Could not open '%s'", file);
  }
  fprintf(out,
          "//\n//\n//\n" // Comment lines are ignored in the diff
                         // but the number of lines has to be identical and the
                         // Modelsim generated files have 3 comment lines.
          " @0 %016lX %016lX %016lX %016lX\n @4 %016lX %016lX %016lX %016lX\n"
          " @8 %016lX %016lX %016lX %016lX\n @c %016lX %016lX %016lX %016lX\n"
          "@10 %016lX %016lX %016lX %016lX\n@14 %016lX %016lX %016lX %016lX\n"
          "@18 %016lX %016lX %016lX %016lX\n@1c %016lX %016lX %016lX %016lX\n",
          node->core.riscv.reg[0], node->core.riscv.reg[1],
          node->core.riscv.reg[2], node->core.riscv.reg[3],
          node->core.riscv.reg[4], node->core.riscv.reg[5],
          node->core.riscv.reg[6], node->core.riscv.reg[7],
          node->core.riscv.reg[8], node->core.riscv.reg[9],
          node->core.riscv.reg[10], node->core.riscv.reg[11],
          node->core.riscv.reg[12], node->core.riscv.reg[13],
          node->core.riscv.reg[14], node->core.riscv.reg[15],
          node->core.riscv.reg[16], node->core.riscv.reg[17],
          node->core.riscv.reg[18], node->core.riscv.reg[19],
          node->core.riscv.reg[20], node->core.riscv.reg[21],
          node->core.riscv.reg[22], node->core.riscv.reg[23],
          node->core.riscv.reg[24], node->core.riscv.reg[25],
          node->core.riscv.reg[26], node->core.riscv.reg[27],
          node->core.riscv.reg[28], node->core.riscv.reg[29],
          node->core.riscv.reg[30], node->core.riscv.reg[31]
          );

  fclose(out);
}
