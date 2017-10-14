/**
 * trace.h
 * Core that sends and receives traces from netrace
 *
 * MacSim project
 */

#ifndef _TRACE_H
#define _TRACE_H

#include "node.h"

void netrace_init_context(node_t *node);
void netrace_finish_context(node_t *node);
void netrace_print_context(node_t *node);
int  netrace_disasm(node_t *node, addr_t pc, char *dstr);
void netrace_print_context(node_t *node);

bool netrace_inject_messages(node_t **nodes);
void netrace_open_file(const char *filename);

#endif
