/**
 * riscv_context.h
 * RISC-V context
 *
 * MacSim project
 */
#ifndef _RISCV_CONTEXT_H
#define _RISCV_CONTEXT_H

#include "share.h"

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
} riscv_context_t;

#endif
