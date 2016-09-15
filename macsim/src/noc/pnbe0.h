/*
 * paternoster_back_up.h
 *
 *  Created on: 24.02.2015
 *      Author: gorlorom
 */

#ifndef _PNBE0_H_
#define _PNBE0_H_

#include "node.h"
#include "share.h"

// Send a message to a core
bool pnbe0_send_packet(node_t *node, rank_t dest, uint_fast32_t len, uint8_t *msg);

// Receive a message from a specified core
bool pnbe0_receive_packet(node_t *node, rank_t src, uint_fast32_t *len, uint8_t *msg);

// Send a flit to a core
bool pnbe0_send_flit_container(node_t *node, rank_t dest, flit_container_t *flit);

// Receive a flit from a specified core
bool pnbe0_receive_flit(node_t *node, rank_t src, uint8_t *msg);

bool pnbe0_sender_ready(node_t *node);

bool pnbe0_probe_rank(node_t *node, rank_t src);

rank_t pnbe0_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnbe0_route_one_cycle(node_t *node);

// initialize the paternoster interconnection simulator (back up version)
void pnbe0_init(node_t *node);

void pnbe0_destroy(node_t *node);

#endif
