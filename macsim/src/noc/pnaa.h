/*
 * gs_all_to_all.h
 *
 *  Created on: 19.03.2015
 *      Author: gorlorom
 */

#ifndef _PNAA_H_
#define _PNAA_H_


#include "node.h"
#include "share.h"

bool pnaa_send_flit(node_t *node, rank_t dest, flit_t flit);
bool pnaa_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool pnaa_sender_ready(node_t *node);

bool pnaa_probe_rank(node_t *node, rank_t src);

rank_t pnaa_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnaa_route_one_cycle(node_t *node);

// initialize the gs_all_to_all interconnection simulator
void pnaa_init(node_t *node);

void pnaa_destroy(node_t *node);


#endif
