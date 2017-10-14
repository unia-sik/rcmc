/**
 * armv3.h
 * ARM v3 ISA (ARM7)
 *
 * MacSim project
 */
#ifndef _ARMV3M_H
#define _ARMV3M_H

#include "node.h"

// Init the memory of the ARM v3
void armv3_init_context(node_t *node);

// Remove context from memory and free memory blocks
void armv3_finish_context(node_t *node);

// Simulate some cycles
bool armv3_simulate(node_t *node, cycle_t cycles);

// Print a register dump of the ARM v3
void armv3_print_context(node_t *node);

// Disassemble one instruction
int armv3_disasm(node_t *node, addr_t pc, char *dstr);


#endif
