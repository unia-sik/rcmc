#ifndef _PERFECT_H
#define _PERFECT_H

#include "share.h"
#include "msgqueue.h"
#include "node.h"

typedef struct {
    msg_queue_t recvbuf;
} perfect_context_t;


bool perfect_send_flit(node_t *node, rank_t dest, flit_t flit);
bool perfect_recv_flit(node_t *node, rank_t src, flit_t *flit);

bool perfect_sender_ready(node_t *node);

bool perfect_probe_rank(node_t *node, rank_t src);

rank_t perfect_probe_any(node_t *node);

void perfect_route_one_cycle(node_t *node);

void perfect_init(node_t *node);

void perfect_destroy(node_t *node);


#endif
