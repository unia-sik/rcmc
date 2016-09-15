/**
 * traffic.h
 * Generator for synthetic traffic patterns
 *
 * MacSim project
 */

#ifndef _TRAFFIC_H
#define _TRAFFIC_H

#include "node.h"

void traffic_init_context(node_t *node);
void traffic_finish_context(node_t *node);
void traffic_print_context(node_t *node);
int  traffic_disasm(node_t *node, addr_t pc, char *dstr);
void traffic_print_context(node_t *node);

#endif
