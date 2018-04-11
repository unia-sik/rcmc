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



// debug NoC: same as perfect, but with debug output

static inline bool debug_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    printf("S %lu->%lu [%lx]\n", node->rank, dest, flit);
    return perfect_send_flit(node, dest, flit);
}

static inline bool debug_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    bool r = perfect_recv_flit(node, src, flit);
    if (r) printf("R %lu->%lu [%lx]\n", src, node->rank, *flit);
    return r;
}

static inline void debug_init(node_t *node)
{
    perfect_init(node);
    node->noc_send_flit         = debug_send_flit;
    node->noc_recv_flit         = debug_recv_flit;
}

static inline void debug_destroy(node_t *node){
    perfect_destroy(node);
}




#endif
