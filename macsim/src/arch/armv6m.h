/**
 * armv6m.h
 * ARM v6-M ISA (Cortex-M0, Cortex-M1)
 *
 * MacSim project
 */
#ifndef _ARMV6M_H
#define _ARMV6M_H

#include "node.h"

#define ARMV6M_SVC_SYSCALL       0x2a

// Exceptions
#define ARMV6M_EXC_SUPERVISOR_CALL 0x0b
#define ARMV6M_EXC_EXTERNAL_IRQ    0x10 //!< map all external exception to this vector!

#define ARMV6M_VECTORTABLE 0x08000000 //!< Base address of exception vector

#define ARMV6M_EXC_RETURN_MASK 0x0ffffff0

/* Device access */
#define ARMV6M_DEV_BASE   0xa0000000
#define ARMV6M_DEV_LENGTH 0x10000000
#define ARMV6M_DEV_MASK   0xf0000000

// Init the memory of the ARM v6-M
void armv6m_init_context(node_t *node);

// Remove context from memory and free memory blocks
void armv6m_finish_context(node_t *node);

// Simulate some cycles
bool armv6m_simulate(node_t *node, cycle_t cycles);

// Print a register dump of the ARM v6-M
void armv6m_print_context(node_t *node);

// Disassemble one instruction
int armv6m_disasm(node_t *node, addr_t pc, char *dstr);


#endif
