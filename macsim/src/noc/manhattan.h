/** 
 * Latency depending on the Manhattan distance, ignoring collisions
 *
 * MacSim project
 */
#ifndef _MANHATTAN_H
#define _MANHATTAN_H

#include "share.h"
#include "flitqueue.h"
#include "node.h"


bool manhattan_send_flit(node_t *node, rank_t dest, flit_t flit);
bool manhattan_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool manhattan_sender_ready(node_t *node);

bool manhattan_probe_rank(node_t *node, rank_t src);

rank_t manhattan_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void manhattan_route_one_cycle(node_t *node);

// Init the interconnection simulator
void manhattan_init(node_t *node);

void manhattan_destroy(node_t *node);

void manhattan_print_context(node_t *nodes[], rank_t max_rank);


#endif
