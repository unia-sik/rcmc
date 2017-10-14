/**
 * armv3_context.h
 * ARM v3 context
 *
 * MacSim project
 */
#ifndef _ARMV3_CONTEXT_H
#define _ARMV3_CONTEXT_H

#include "share.h"

typedef struct
{
    // registers
    uint32_t reg[16];
    bool apsr_n;
    bool apsr_z;
    bool apsr_c;
    bool apsr_v;

    uint32_t ipsr; // interrupt service routine number (when in exception)
    bool control_npriv;
    bool control_spsel; // 0 use SP_main as current stack, 1 use SP_process in thread mode
    bool primask_pm;
    uint32_t sp_process;
    uint32_t sp_main;
    uint32_t mode;

    // management / debugging
    uint32_t nextpc; // address of next instruction
    cycle_t memmult_latency; // latency of a multiple memory access
    // (IC_ARMV3_MEMMULT or _RETMULT)
} armv3_context_t;

#endif // _ARMV3M_CONTEXT_H
