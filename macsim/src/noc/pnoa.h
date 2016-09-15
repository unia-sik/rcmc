/*
 * gs_one_to_all.h
 *
 *  Created on: 16.03.2015
 *      Author: gorlorom
 */

#ifndef _PNOA_H_
#define _PNOA_H_


#include "node.h"
#include "share.h"

bool pnoa_send_flit_container(node_t *node, rank_t dest, flit_t flit);
bool pnoa_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool pnoa_sender_ready(node_t *node);

bool pnoa_probe_rank(node_t *node, rank_t src);

rank_t pnoa_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnoa_route_one_cycle(node_t *node);

// initialize the gs_one_to_all interconnection simulator
void pnoa_init(node_t *node);

void pnoa_destroy(node_t *node);


#endif
