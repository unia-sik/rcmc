/* fgmp.h[6~
 * Fine-graind message passing interface to the NoC 
 *
 * for ARMv6-M (Cortex-M0)
 */

#ifndef _FGMP_H
#define _FGMP_H

#include <assert.h>
#include <stdint.h>

typedef int32_t cid_t;
typedef uint64_t coordinates_t;
typedef uint32_t flit_t;


static inline cid_t fgmp_get_max_cid()
{
    cid_t max_cid;
    asm volatile (
        "svc 80\n\t"
        "mov %0, r0\n\t"
        : "=r" (max_cid) : : "r0");
    return max_cid;
}



static inline cid_t fgmp_get_cid()
{
    cid_t cid;
    asm volatile (
	"svc 81\n\t"
	"mov %0, r0\n\t"
	: "=r" (cid) : : "r0");
    return cid;
}


// Get the physical topology
// Stored as vector of 4 16 bit values: maximum number of nodes per dimension
static inline coordinates_t fgmp_get_max_coordinates()
{
    assert(!"Not yet implemented");
}



static inline void fgmp_send_flit(cid_t dest, flit_t flit)
{
    asm volatile (
	"mov r0, %0\n\t"
	"mov r1, %1\n\t"
	"svc 82\n\t"
	: : "r" (dest), "r" (flit) : "r0", "r1");
}


static inline flit_t fgmp_recv_flit(cid_t source)
{
    flit_t flit;
    asm volatile (
	"mov r0, %1\n\t"
	"svc 83\n\t"
	"mov %0, r0\n\t"
	: "=r" (flit) : "r" (source) : "r0");
    return flit;
}


// congestion: send buffer full?
static inline int fgmp_cong()
{
    int flag;
    asm volatile (
        "svc 84\n\t"
        "mov %0, r0\n\t"
        : "=r" (flag) : : "r0");
    return flag;
}

/* DEPRECATED

// wait until a flit from any core arrived
static inline cid_t fgmp_wait()
{
    cid_t sender;
    asm volatile (
        "svc 85\n\t"
        "mov %0, r0\n\t"
        : "=r" (sender) : : "r0");
    return sender;
}
*/

// something received from specific core?
static inline int fgmp_probe(cid_t source)
{
    int flag;
    asm volatile (
	"mov r0, %1\n\t"
	"svc 86\n\t"
	"mov %0, r0\n\t"
	: "=r" (flag) : "r" (source) : "r0");
    return flag;
}


// something received from any core?
static inline int fgmp_any()
{
    cid_t rank;
    asm volatile (
        "svc 87\n\t"
        "mov %0, r0\n\t"
        : "=r" (rank) : : "r0");
    return rank;
}


// architecture independent


static inline cid_t fgmp_cid_from_xyz(uint_fast16_t u, uint_fast16_t z, 
    uint_fast16_t y, uint_fast16_t x)
{
    coordinates_t max = fgmp_get_max_coordinates();
    uint_fast16_t zm = (max >> 32) & 0xffff;
    uint_fast16_t ym = (max >> 16) & 0xffff;
    uint_fast16_t xm = max & 0xffff;
    return (((u*zm)+z)*ym+y)*xm+x;
}


static inline cid_t fgmp_cid_from_coordinates(coordinates_t c)
{
    return fgmp_cid_from_xyz((c>>48)&0xffff, (c>>32)&0xffff, (c>>16)&0xffff, c&0xffff);
}


static inline coordinates_t fgmp_coordinates_from_cid(cid_t c)
{
    coordinates_t max = fgmp_get_max_coordinates();
    uint_fast16_t zm = (max >> 32) & 0xffff;
    uint_fast16_t ym = (max >> 16) & 0xffff;
    uint_fast16_t xm = max & 0xffff;
    coordinates_t x = c % xm;
    c = c / xm;
    coordinates_t y = c % ym;
    c = c / ym;
    coordinates_t z = c % zm;
    c = c / zm;
    return ((coordinates_t)(c/zm)<<48) | (z<<32) | (y<<16) | x;
}


#endif

