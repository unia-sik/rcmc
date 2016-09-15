/*
 * best_effort_prototype1.h
 *
 *  Created on: 09.04.2015
 *      Author: gorlorom
 */

#ifndef BEST_EFFORT_PROTOTYPE1_H_
#define BEST_EFFORT_PROTOTYPE1_H_


#include "node.h"
#include "share.h"

// Send a message to a core
bool pnbe1_send_packet(node_t *node, rank_t dest, uint_fast32_t len, uint8_t *msg);

// Receive a message from a specified core
bool pnbe1_receive_packet(node_t *node, rank_t src, uint_fast32_t *len, uint8_t *msg);

// Send a flit to a core
bool pnbe1_send_flit_container(node_t *node, rank_t dest, flit_container_t *flit);

// Receive a flit from a specified core
bool pnbe1_receive_flit(node_t *node, rank_t src, uint8_t *msg);

bool pnbe1_send_flit(node_t *node, rank_t dest, flit_t flit);
bool pnbe1_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool pnbe1_sender_ready(node_t *node);

bool pnbe1_probe_rank(node_t *node, rank_t src);

rank_t pnbe1_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnbe1_route_one_cycle(node_t *node);

// initialize the best effort prototype 1 interconnection simulator
void pnbe1_init(node_t *node);

void pnbe1_destroy(node_t *node);




#endif /* BEST_EFFORT_PROTOTYPE1_H_ */
