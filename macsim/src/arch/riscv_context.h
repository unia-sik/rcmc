/**
 * riscv_context.h
 * RISC-V context
 *
 * MacSim project
 */
#ifndef _RISCV_CONTEXT_H
#define _RISCV_CONTEXT_H

#include "share.h"

#define RISCV_LOADBUF_SIZE      32      // size of a cacheline

typedef struct
{
    // registers
    int64_t     reg[32];
    double      freg[32];

    // core special registers
    uint64_t    fcsr;


    uint64_t    mstatus;
    addr_t      mtvec;
    uint64_t    mscratch;
    addr_t      mepc;
    uint64_t    mcause;


    // only for rvmpb:
    // Unused for riscv, but the additional memory is so low that separating 
    // it isn't worth it.

    // simple buffer for the last MPB cache line
    addr_t      loadbuf_rank; // core id of the MPB, invalid if higher
    addr_t      loadbuf_base; // address of the 32 byte cache line in the MPB
    addr_t      loadbuf_pos;  // position when filling the load buffer
    uint8_t     loadbuf[RISCV_LOADBUF_SIZE];

    addr_t      mpb_send_rank;
    addr_t      mpb_send_base;
    addr_t      mpb_send_pos;
} riscv_context_t;

#endif
