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

#endif
