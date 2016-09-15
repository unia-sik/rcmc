/*
 * pnbe2.h
 *
 *  Created on: 02.04.2015
 *      Author: gorlorom
 */

#ifndef PATERNOSTER_BEST_EFFORT_H_
#define PATERNOSTER_BEST_EFFORT_H_

#include "node.h"
#include "share.h"


bool pnbe2_send_flit(node_t *node, rank_t dest, flit_t flit);
bool pnbe2_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool pnbe2_sender_ready(node_t *node);

bool pnbe2_probe_rank(node_t *node, rank_t src);

rank_t pnbe2_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnbe2_route_one_cycle(node_t *node);

// initialize the best effort paternoster interconnection simulator
void pnbe2_init(node_t *node);

void pnbe2_destroy(node_t *node);


#endif /* PATERNOSTER_BEST_EFFORT_H_ */
