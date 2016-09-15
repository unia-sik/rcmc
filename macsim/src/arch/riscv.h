/**
 * riscv.h
 * RISC-V ISA
 *
 * MacSim project
 */
#ifndef _RISCV_H
#define _RISCV_H

#include "node.h"

// Device access (currently unused)
#define RISCV_DEV_BASE   0xa0000000
#define RISCV_DEV_LENGTH 0x10000000
#define RISCV_DEV_MASK   0xf0000000

// Init the memory of the ARM v6-M
void riscv_init_context(node_t *node);

// Remove context from memory and free memory blocks
void riscv_finish_context(node_t *node);

// Print a register dump of the ARM v6-M
void riscv_print_context(node_t *node);

// Dump the current context to file.
void riscv_dump_context(const char *file, node_t *node);

// Disassemble one instruction
int  riscv_disasm(node_t *node, addr_t pc, char *dstr);


#endif
