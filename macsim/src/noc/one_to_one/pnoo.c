#include "pnoo.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "share.h"

#include "pnoo_helper.h"
#include "pnoo_context.h"
#include "pnoo_timing.h"

extern node_t* nodes[];

pnoo_context_t* pnoo_get_context(node_t* node)
{
    return (pnoo_context_t*) node->noc_context;
}

bool pnoo_send_flit(node_t* node, rank_t dest, flit_t flit)
{    
    dest = (dest & 0xFFFF) + ((dest & 0xFFFF0000) >> 16) * conf_noc_width;
    pnoo_context_t* context = pnoo_get_context(node);
    pnoo_msg_t msg = pnoo_msg_init_with_data(node->rank, dest, flit);   
    
    if (context->num_sendbuffer_entries == PNOO_CONTEXT_SENDBUFFER_SIZE) {
        return false;
    }

    context->num_sendbuffer_entries++;
    
    pnoo_event_queue_item_t* lastEvent = pnoo_event_queue_search(&context->eventQueue, PNOO_EVENT_SEND_BUFFER_REMOVE);
        
    if (lastEvent == NULL) {
        cycle_t sendTime = pnoo_timing_calc_access_time(node);
        pnoo_context_add_timed_event(context, sendTime, PNOO_EVENT_SEND_BUFFER_REMOVE, &msg, false);
    } else {
        pnoo_event_queue_insert(&context->eventQueue, lastEvent->clk, PNOO_EVENT_SEND_BUFFER_REMOVE, &msg);
    }

    return true;
}

bool pnoo_recv_flit(node_t* node, rank_t src, flit_t* flit)
{
    src = (src & 0xFFFF) + ((src & 0xFFFF0000) >> 16) * conf_noc_width;
    pnoo_context_t* context = pnoo_get_context(node);

    int index = pnoo_recv_buffer_search_from_rank(&context->recvBuffer, src);

    if (index != -1) {
        *flit = pnoo_recv_buffer_get_flit(&context->recvBuffer, index);
        pnoo_recv_buffer_erase(&context->recvBuffer, index);
        context->num_recvbuffer_entries--;

        cycle_t sendTime = pnoo_timing_calc_access_time(node);

        if (PNOO_ENABLE_FLOW_CONTROL && context->num_recvbuffer_entries == PNOO_CONTEXT_RECVBUFFER_SIZE - PNOO_FLOW_CONTROL_SIZE - 1) {
            pnoo_context_t* contextSrc = pnoo_get_context(nodes[context->srdyDest]);
            pnoo_msg_t msg = pnoo_msg_init(node->rank, context->srdyDest);
            pnoo_context_add_timed_event(contextSrc, sendTime, PNOO_EVENT_RDY_RECV_COMMIT, &msg, true);
        }

        return true;
    }

    return false;
}

bool pnoo_sender_ready(node_t* node)
{
    pnoo_context_t* context = pnoo_get_context(node);
    return context->num_sendbuffer_entries < PNOO_CONTEXT_SENDBUFFER_SIZE;
}

bool pnoo_send_ready(node_t* node, rank_t dest)
{
    dest = (dest & 0xFFFF) + ((dest & 0xFFFF0000) >> 16) * conf_noc_width;
    pnoo_context_t* context = pnoo_get_context(node);
    pnoo_context_t* contextDest = pnoo_get_context(nodes[dest]);
    cycle_t sendTime = pnoo_timing_calc_access_time(node);
    context->srdyDest = dest;

    rank_t excludeRank = -1;
    if (context->num_recvbuffer_entries < PNOO_CONTEXT_RECVBUFFER_SIZE - PNOO_FLOW_CONTROL_SIZE || !PNOO_ENABLE_FLOW_CONTROL) {
        pnoo_msg_t msg = pnoo_msg_init(node->rank, dest);
        pnoo_context_add_timed_event(contextDest, sendTime, PNOO_EVENT_RDY_RECV, &msg, false);
        pnoo_context_add_timed_event(contextDest, sendTime, PNOO_EVENT_RDY_RECV_COMMIT, &msg, false);
        excludeRank = dest;
    }
    
    for (int i = 0; i < conf_max_rank; i++) {
        if (i != excludeRank && i != node->rank) {
            pnoo_context_t* contextI = pnoo_get_context(nodes[i]);
            pnoo_msg_t msgI = pnoo_msg_init(node->rank, i);
            pnoo_context_add_timed_event(contextI, sendTime, PNOO_EVENT_RDY_RECV_RELEASE, &msgI, false);
        }
    }

    return true;
}

bool pnoo_dest_ready(node_t* node, rank_t src)
{
    src = (src & 0xFFFF) + ((src & 0xFFFF0000) >> 16) * conf_noc_width;
    pnoo_context_t* context = pnoo_get_context(node);
    bool result = pnoo_bit_array_check(&context->bitArray, src);
    pnoo_bit_array_unset(&context->bitArray, src);
    return result;
}

bool pnoo_probe_rank(node_t* node, rank_t src)
{
    fatal("probe_rank is not supported");
    return false;
}

rank_t pnoo_probe_any(node_t* node)
{
    pnoo_context_t* context = pnoo_get_context(node);
    int index = pnoo_recv_buffer_search_from_any(&context->recvBuffer);
    rank_t rank = pnoo_recv_buffer_get_src(&context->recvBuffer, index);
    return ((rank / conf_noc_width) << 16) | (rank % conf_noc_width);
}


void pnoo_init_barrier(node_t* node, rank_t start, rank_t end) {
    pnoo_context_t* context = pnoo_get_context(node);
    context->barrierMin = start;
    context->barrierMax = end;
    context->barrierCount = 0;
    context->barrierCurrent = 0;
    
    cycle_t sendTime = pnoo_timing_calc_access_time(node);
    
    pnoo_msg_t msg = pnoo_msg_init(node->rank, node->rank);
    pnoo_context_add_timed_event(context, sendTime, PNOO_EVENT_BARRIER_ENABLE, &msg, true);    
    
    for (int x = (start & 0xFFFF); x <= (end & 0xFFFF); x++) {
        for (int y = ((start & 0xFFFF0000) >> 16); y <= ((end & 0xFFFF0000) >> 16); y++) {
            context->barrierCount++;
        }    
    }
    
    
}

bool pnoo_check_barrier(node_t* node) {
    pnoo_context_t* context = pnoo_get_context(node);

    if (context->barrierCount == context->barrierCurrent && node->cycle + 2 != context->barrierLastAccess) {
        rank_t addr = ((node->rank / conf_noc_width) << 16) | (node->rank % conf_noc_width);
        context->barrierMin = addr;
        context->barrierMax = addr;
        context->barrierCount = 0;
        context->barrierCurrent = 0;
        context->barrierEnable = false;
        
        return true;
    }
    
    return false;
}

void pnoo_route_one_cycle(node_t* node)
{
    pnoo_context_t* context = pnoo_get_context(node);    
    pnoo_event_queue_execute(&context->eventQueue, pnoo_timing_calc_access_time(node));
}

void pnoo_init(node_t* node)
{
    node->noc_send_flit         = pnoo_send_flit;
    node->noc_recv_flit         = pnoo_recv_flit;

    node->noc_send_ready        = pnoo_send_ready;
    node->noc_sender_ready      = pnoo_sender_ready;
    node->noc_dest_ready        = pnoo_dest_ready;

    node->noc_init_barrier      = pnoo_init_barrier;
    node->noc_check_barrier     = pnoo_check_barrier;

    node->noc_probe_rank        = pnoo_probe_rank;
    node->noc_probe_any         = pnoo_probe_any;
    node->noc_route_one_cycle   = pnoo_route_one_cycle;

    // add NoC specific fields to the node
    pnoo_context_t* context = malloc(sizeof(pnoo_context_t));

    if (context == NULL) {
        fatal("Malloc failed to allocate memory\n");
    }

    node->noc_context = context;
    pnoo_context_init(context);
}

void pnoo_destroy(node_t* node)
{
    pnoo_context_destroy(pnoo_get_context(node));
    free(node->noc_context);
}

