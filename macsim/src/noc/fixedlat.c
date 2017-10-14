/** 
 * $Id: wwabstract.c 509 2013-04-08 22:17:56Z mischejo $
 * Fixed latency abstraction of the NoC
 *
 * McSim project
 */
#include "fixedlat.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//#define DEBUG printf
#define DEBUG(...)

#define FIXEDLAT_MAX_SEND_FIFO	8	// Size of the send buffer of each core

extern node_t *nodes[];


typedef struct {
    msg_queue_t fixedlat_incomming;
    msg_queue_t fixedlat_intransit[FIXEDLAT_TRANSPORT_DELAY];
    msg_queue_t fixedlat_arrived;
    uint_fast32_t fixedlat_intransit_pos;
} fixedlat_context_t;


// Send a flit to a core
bool fixedlat_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    DEBUG("S %lu->%lu [%lx]\n", node->rank, dest, flit);

    if (((fixedlat_context_t *)nodes[dest]->noc_context)->fixedlat_incomming.count >= FIXEDLAT_MAX_SEND_FIFO)
        return 0; // send fifo full
    if (!msg_enqueue(&((fixedlat_context_t *)nodes[dest]->noc_context)->fixedlat_incomming, 
        (uint8_t *)&flit, FLIT_LEN, node->rank))
            fatal("Out of memory");
    return 1;
}


// Receive a flit from a specified core
bool fixedlat_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    uint32_min_t l;
    bool r = msg_dequeue_rank(&((fixedlat_context_t *)node->noc_context)->fixedlat_arrived, 
        (uint8_t *)flit, &l, src);
    if (r) DEBUG("R %lu->%lu [%lx]\n", src, node->rank, *flit);
    return r;
}


bool fixedlat_sender_ready(node_t *node)
{
    return (((fixedlat_context_t *)node->noc_context)->fixedlat_incomming.count 
        < FIXEDLAT_MAX_SEND_FIFO);
}


bool fixedlat_probe_rank(node_t *node, rank_t src)
{
    msg_queue_entry_t *h=((fixedlat_context_t *)node->noc_context)
        ->fixedlat_arrived.head;
    while (h) {
        if (h->rank == src) return true;
        h = h->next;
    }
    return false;
}


rank_t fixedlat_probe_any(node_t *node)
{
    return (((fixedlat_context_t *)node->noc_context)->fixedlat_arrived.head==0)
            ? (rank_t)-1
            : ((fixedlat_context_t *)node->noc_context)->fixedlat_arrived.head->rank;
}


// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void fixedlat_route_one_cycle(node_t *node)
{
	fixedlat_context_t *fnode = (fixedlat_context_t *)node->noc_context;
    // move messages from last transit buffer to arrived fifo
    msg_cat_queue(&fnode->fixedlat_arrived,
    		&fnode->fixedlat_intransit[fnode->fixedlat_intransit_pos]);
    // move messages from incoming to first transit buffer
    ((fixedlat_context_t *)node->noc_context)->fixedlat_intransit[((fixedlat_context_t *)node->noc_context)->fixedlat_intransit_pos] =
        ((fixedlat_context_t *)node->noc_context)->fixedlat_incomming;
    // clear incoming buffer
    msg_init_queue(&((fixedlat_context_t *)node->noc_context)->fixedlat_incomming);
    // turn ring buffer to next position
    ((fixedlat_context_t *)node->noc_context)->fixedlat_intransit_pos++;
    if (((fixedlat_context_t *)node->noc_context)->fixedlat_intransit_pos >= FIXEDLAT_TRANSPORT_DELAY)
        ((fixedlat_context_t *)node->noc_context)->fixedlat_intransit_pos = 0;
}


// Init the interconnection simulator
void fixedlat_init(node_t *node)
{
    uint16_min_t p;

    // set function pointers
    node->noc_send_flit         = fixedlat_send_flit;
    node->noc_recv_flit         = fixedlat_recv_flit;
    node->noc_sender_ready      = fixedlat_sender_ready;
    node->noc_probe_rank        = fixedlat_probe_rank;
    node->noc_probe_any         = fixedlat_probe_any;
    node->noc_route_one_cycle   = fixedlat_route_one_cycle;
    fixedlat_context_t *context = malloc(sizeof(fixedlat_context_t));
    if(context == NULL){
        fatal("Malloc failed to allocate memory");
    }
    node->noc_context = context;

    msg_init_queue(&((fixedlat_context_t *)node->noc_context)->fixedlat_incomming);
    msg_init_queue(&((fixedlat_context_t *)node->noc_context)->fixedlat_arrived);
    for (p=0; p<FIXEDLAT_TRANSPORT_DELAY; p++)
        msg_init_queue(&((fixedlat_context_t *)node->noc_context)->fixedlat_intransit[p]);
    ((fixedlat_context_t *)node->noc_context)->fixedlat_intransit_pos = 0;
}

void fixedlat_destroy(node_t *node){
    free(node->noc_context);
}
