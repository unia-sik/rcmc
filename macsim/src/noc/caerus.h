/*
 * caerus.h
 *
 *  Created on: 31.03.2015
 *      Author: gorlorom
 */

#ifndef CAERUS_H_
#define CAERUS_H_

#include "node.h"
#include "share.h"

// Send a message to a core
bool caerus_send_packet(node_t *node, rank_t dest, uint_fast32_t len, uint8_t *msg);

// Receive a message from a specified core
bool caerus_receive_packet(node_t *node, rank_t src, uint_fast32_t *len, uint8_t *msg);

// Send a flit to a core
bool caerus_send_flit_container(node_t *node, rank_t dest, flit_container_t *flit);

// Receive a flit from a specified core
bool caerus_receive_flit(node_t *node, rank_t src, uint8_t *msg);

bool caerus_send_flit(node_t *node, rank_t dest, flit_t flit);
bool caerus_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool caerus_sender_ready(node_t *node);

bool caerus_probe_rank(node_t *node, rank_t src);

rank_t caerus_probe_any(node_t *node);

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void caerus_route_one_cycle(node_t *node);

// initialize the caerus interconnection simulator
void caerus_init(node_t *node);

void caerus_destroy(node_t *node);

#endif /* CAERUS_H_ */
