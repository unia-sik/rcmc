/* fgmp.h
 * Fine-graind message passing interface to the NoC 
 *
 * for RISC-V RV64I
 */
#ifndef _FGMP_H
#define _FGMP_H


// uncomment to use newer PIMP-2 instructions
//#define NEW_PIMP

// set if assembler supports FGMP instructions directly:
#define ASSEMBLER_SUPPORT

#define FGMP_MAX_CID 256


#include <assert.h>
#include <stdint.h>


#define RISCV_CSR_MAXCID   0xC70
#define RISCV_CSR_CID      0xC71
#define RISCV_CSR_NOCDIM   0xC72

typedef int64_t cid_t;
typedef uint64_t coordinates_t;
typedef uint64_t flit_t;

#define CSR_NAME(A) #A
#define riscv_read_csr(__regname) ({ uint64_t __t; \
    asm volatile ("csrr %0, " CSR_NAME(__regname) : "=r"(__t)); __t; })





// Number of nodes
static inline cid_t fgmp_get_max_cid()
{
    return riscv_read_csr(RISCV_CSR_MAXCID);
}


// Individual number of the current core
static inline cid_t fgmp_get_cid()
{
    return riscv_read_csr(RISCV_CSR_CID);
}


// Get the physical topology
// Stored as vector of 4 16 bit values: maximum number of nodes per dimension
static inline coordinates_t fgmp_get_nocdim()
{
    return riscv_read_csr(RISCV_CSR_NOCDIM);
}







// ---------------------------------------------------------------------
// new PIMP
// ---------------------------------------------------------------------

#ifdef NEW_PIMP

static inline void fgmp_recv_wait()
{
#ifdef ASSEMBLER_SUPPORT
    asm volatile ("1: bre 1b\n\t" : : );
#else
#endif
}


static inline cid_t fgmp_recv_node()
{
    cid_t sender;
#ifdef ASSEMBLER_SUPPORT
    asm volatile ( "rcvn %0\n\t" : "=r" (sender) : );
#else
#endif
    return sender;
}


static inline flit_t fgmp_recv_payload()
{
    flit_t flit;
#ifdef ASSEMBLER_SUPPORT
    asm volatile ( "rcvp %0\n\t" : "=r" (flit) : );
#else
#endif
    return flit;
}


static inline long fgmp_recv_empty()
{
    long flag;
#ifdef ASSEMBLER_SUPPORT
    asm volatile (
        "li %0, 0\n\t"
        "brne 1f\n\t"
        "li %0, 1\n\t"
        "1:\n\t"
        : "=r" (flag));
#else
#endif
    return flag;
}


// ---------------------------------------------------------------------
// wrapper for old PIMP
// ---------------------------------------------------------------------


extern flit_t fgmp_first_flit[FGMP_MAX_CID];
extern char fgmp_flit_buffered[FGMP_MAX_CID];


static inline long fgmp_cong()
{
    long flag;
#ifdef ASSEMBLER_SUPPORT
    asm volatile (
        "li %0, 1\n\t"
        "bsf 1f\n\t"
        "li %0, 0\n\t"
        "1:\n\t"
        : "=r" (flag));
#endif
    return flag;
}


static inline cid_t fgmp_any()
{
    int_fast32_t cid;
#ifdef ASSEMBLER_SUPPORT
   // First the receive buffer is checked, then the buffer in the memory.
   // Therefore the returned CID may not be the CID of the oldest flit,
   // but this should not be a problem.

    asm volatile (
        "li %0, -1\n\t"
        "bre 1f\n\t"
        "rcvn %0\n\t"
        "1:\n\t"
        : "=r" (cid));
#endif
    if (cid<0) {
        cid_t n = fgmp_get_max_cid();
        for (cid=0; cid<n; cid++) {
            if (fgmp_flit_buffered[cid]) return cid;
        }
        cid = -1;
    }
    return cid;
}


static inline void fgmp_send_flit(cid_t dest, flit_t flit)
{
#ifdef ASSEMBLER_SUPPORT
//    asm volatile ( "send %0, %1\n\t" : : "r" (dest), "r" (flit) );
    asm volatile (
        "1: bsf 1b\n\t"
        "snd %0, %1\n\t" 
        : : "r" (dest), "r" (flit) );
#else
#endif
}


static inline flit_t fgmp_recv_flit(cid_t source)
{
    if (fgmp_flit_buffered[source]) {
        fgmp_flit_buffered[source] = 0;
        return fgmp_first_flit[source];
    }

    while (1) {
        fgmp_recv_wait();
        cid_t sender = fgmp_recv_node();
        if (sender == source) {
            return fgmp_recv_payload();
        }
        assert (fgmp_flit_buffered[sender]==0);
        fgmp_flit_buffered[sender] = 1;
        fgmp_first_flit[sender] = fgmp_recv_payload();
    }
}


static inline long fgmp_probe(cid_t source)
{
    if (fgmp_flit_buffered[source]) {
        return 1;
    }

    while (!fgmp_recv_empty()) {
        cid_t sender = fgmp_recv_node();
        if (sender == source) {
            return 1;
        }
        assert (fgmp_flit_buffered[sender]==0);
        fgmp_flit_buffered[sender] = 1;
        fgmp_first_flit[sender] = fgmp_recv_payload();
    }
    return 0;
}





#else

// ---------------------------------------------------------------------
// new PIMP
// ---------------------------------------------------------------------


// Is send buffer full?
static inline long fgmp_cong()
{
    long flag;
#ifdef ASSEMBLER_SUPPORT
    asm volatile (
    "cong %0"
    : "=r" (flag));
#else
    asm volatile (
    ".word 0x0000156b\n\t" // cong a0
    "mv %0, a0\n\t"
    : "=r" (flag) : : "a0");
#endif
    return flag;
}


// Has any flit arrived
static inline cid_t fgmp_any()
{
    int_fast32_t cid;
#ifdef ASSEMBLER_SUPPORT
    asm volatile ("any %0"
    : "=r" (cid));
#else
    asm volatile (
    ".word 0x0000756b\n\t" // any a0
    "mv %0, a0\n\t"
    : "=r" (cid) : : "a0");
#endif
    return cid;
}


static inline void fgmp_send_flit(cid_t dest, flit_t flit)
{
#ifdef ASSEMBLER_SUPPORT
    asm volatile ( "send %0, %1\n\t" : : "r" (dest), "r" (flit) );
#else
    asm volatile ( 
	"mv a0, %0\n\t"
	"mv a1, %1\n\t"
	".word 0x00b5006b\n\t" // send a0, a1	        
	: : "r" (dest), "r" (flit) : "a0", "a1");
#endif
}


static inline flit_t fgmp_recv_flit(cid_t source)
{
    flit_t flit;
#ifdef ASSEMBLER_SUPPORT
    asm volatile ( "recv %0, %1\n\t" : "=r" (flit) : "r" (source) );
#else
    asm volatile (
	"mv a0, %1\n\t"
	".word 0x0005456b\n\t" // recv a0, a0
	"mv %0, a0\n\t"
	: "=r" (flit) : "r" (source) : "a0");
#endif
    return flit;
}


static inline long fgmp_probe(cid_t source)
{
    long flag;
#ifdef ASSEMBLER_SUPPORT
    asm volatile ( "probe %0, %1\n\t" : "=r" (flag) : "r" (source) );
#else
    asm volatile (
	"mv a0, %1\n\t"
	".word 0x0005556b\n\t" // probe a0, a0
	"mv %0, a0\n\t"
	: "=r" (flag) : "r" (source) : "a0");
#endif
    return flag;
}

#endif





// ---------------------------------------------------------------------
// architecture independent
// ---------------------------------------------------------------------


static inline cid_t fgmp_cid_from_xyz(uint_fast16_t u, uint_fast16_t z, 
    uint_fast16_t y, uint_fast16_t x)
{
    coordinates_t max = fgmp_get_nocdim();
    uint_fast16_t zm = (max >> 32) & 0xffff;
    uint_fast16_t ym = (max >> 16) & 0xffff;
    uint_fast16_t xm = max & 0xffff;
    return (((u*zm)+z)*ym+y)*xm+x;
}


static inline cid_t fgmp_cid_from_coordinates(coordinates_t c)
{
    return fgmp_cid_from_xyz((c>>48)&0xffff, (c>>32)&0xffff, (c>>16)&0xffff, c&0xffff);
}


static inline coordinates_t fgmp_coordinates_from_cid(cid_t r)
{
    coordinates_t max = fgmp_get_nocdim();
    uint_fast16_t zm = (max >> 32) & 0xffff;
    uint_fast16_t ym = (max >> 16) & 0xffff;
    uint_fast16_t xm = max & 0xffff;
    coordinates_t x = r % xm;
    r = r / xm;
    coordinates_t y = r % ym;
    r = r / ym;
    coordinates_t z = r % zm;
    r = r / zm;
    return ((coordinates_t)(r/zm)<<48) | (z<<32) | (y<<16) | x;
}

#endif
