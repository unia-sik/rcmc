/** 
 * $Id: wwabstract.c 509 2013-04-08 22:17:56Z mischejo $
 * Fixed latency abstraction of the NoC
 *
 * McSim project
 */
#include "perfect.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



extern node_t *nodes[];


// Send a flit to a core
bool perfect_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    if (!msg_enqueue(&((perfect_context_t *)nodes[dest]->noc_context)->recvbuf, 
        (uint8_t *)&flit, FLIT_LEN, node->rank))
            fatal("Out of memory");
    return 1;
}


// Receive a flit from a specified core
bool perfect_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    uint32_min_t l;
    bool r = msg_dequeue_rank(&((perfect_context_t *)node->noc_context)->recvbuf, 
        (uint8_t *)flit, &l, src);
    return r;
}


bool perfect_sender_ready(node_t *node)
{
    return true;
}


bool perfect_probe_rank(node_t *node, rank_t src)
{
    msg_queue_entry_t *h=((perfect_context_t *)node->noc_context)->recvbuf.head;
    while (h) {
        if (h->rank == src) return true;
        h = h->next;
    }
    return false;
}


rank_t perfect_probe_any(node_t *node)
{
    return (((perfect_context_t *)node->noc_context)->recvbuf.head==0)
            ? (rank_t)-1
            : ((perfect_context_t *)node->noc_context)->recvbuf.head->rank;
}


void perfect_route_one_cycle(node_t *node)
{
}


// Init the interconnection simulator
void perfect_init(node_t *node)
{
    // set function pointers
    node->noc_send_flit         = perfect_send_flit;
    node->noc_recv_flit         = perfect_recv_flit;
    node->noc_sender_ready      = perfect_sender_ready;
    node->noc_probe_rank        = perfect_probe_rank;
    node->noc_probe_any         = perfect_probe_any;
    node->noc_route_one_cycle   = perfect_route_one_cycle;
    perfect_context_t *context = malloc(sizeof(perfect_context_t));
    if(context == NULL){
        fatal("Malloc failed to allocate memory");
    }
    node->noc_context = context;

    msg_init_queue(&((perfect_context_t *)node->noc_context)->recvbuf);
}


void perfect_destroy(node_t *node){
    free(node->noc_context);
}
