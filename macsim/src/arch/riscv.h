/**
 * riscv.h
 * RISC-V ISA
 *
 * MacSim project
 */
#ifndef _RISCV_H
#define _RISCV_H

#include "node.h"

// Init the memory
void riscv_init_context(node_t *node);

// Remove context from memory and free memory blocks
void riscv_finish_context(node_t *node);

// Print a register dump
void riscv_print_context(node_t *node);

// Dump the current context to file.
void riscv_dump_context(const char *file, node_t *node);

// Disassemble one instruction
int  riscv_disasm(node_t *node, addr_t pc, char *dstr);


// internal use, only in riscv.c and rvmpb.c
bool riscv_instruction_uses_reg_s(uint_fast32_t iw);
bool riscv_instruction_uses_reg_t(uint_fast32_t iw);
instruction_class_t riscv_execute_iw(node_t *node, uint_fast32_t iw, 
    uint_fast32_t next_iw);

#define RISCV_CANONICAL_NAN_FLOAT       0x7FC00000

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

#endif
