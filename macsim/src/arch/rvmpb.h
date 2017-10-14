/**
 * rvmpb.h
 * RISC-V ISA with message passing buffers
 *
 * MacSim project
 */
#ifndef _RVMPB_H
#define _RVMPB_H

#include "node.h"

// Init the memory
void rvmpb_init_context(node_t *node);

// Remove context from memory and free memory blocks
void rvmpb_finish_context(node_t *node);

#endif
