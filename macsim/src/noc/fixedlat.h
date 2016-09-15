/** 
 * $Id: wwabstract.h 509 2013-04-08 22:17:56Z mischejo $
 * Abstract implementation of Water Wheel Routing
 *
 * McSim project
 */
#ifndef _FIXEDLAT_H
#define _FIXEDLAT_H

#include "share.h"
#include "msgqueue.h"
#include "node.h"

#define FIXEDLAT_TRANSPORT_DELAY	3	// #Rounds a msg is within the interconnect


bool fixedlat_send_flit(node_t *node, rank_t dest, flit_t flit);
bool fixedlat_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool fixedlat_sender_ready(node_t *node);

bool fixedlat_probe_rank(node_t *node, rank_t src);

rank_t fixedlat_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void fixedlat_route_one_cycle(node_t *node);

// Init the interconnection simulator
void fixedlat_init(node_t *node);

void fixedlat_destroy(node_t *node);


#endif // _FIXEDLAT_H
