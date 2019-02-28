#include "pnoo_events_recv.h"
#include "share.h"
#include "node.h"
#include "pnoo.h"
#include "pnoo_timing.h"
#include "pnoo_helper.h"

extern node_t* nodes[];

void pnoo_events_recv_init(pnoo_event_queue_t* queue, pnoo_context_t* context)
{
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_RECV_BUFFER_INSERT, (pnoo_event_queue_callback_t) pnoo_events_recv_insert_callback, context);
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_RECV_BUFFER_INCREMENT, (pnoo_event_queue_callback_t) pnoo_events_recv_increment_callback, context);    
}

bool pnoo_events_recv_insert_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_recv_buffer_push(&context->recvBuffer, msg);    
    return true;
}

bool pnoo_events_recv_increment_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    context->num_recvbuffer_entries++;
    if (PNOO_ENABLE_FLOW_CONTROL && context->num_recvbuffer_entries >= PNOO_CONTEXT_RECVBUFFER_SIZE - PNOO_FLOW_CONTROL_SIZE) {     
        pnoo_context_t* contextSrc = (pnoo_context_t*) nodes[context->srdyDest]->noc_context;
        pnoo_msg_t outMsg = pnoo_msg_init(msg->flit.dest, context->srdyDest);
        pnoo_context_add_timed_event(contextSrc, clk, PNOO_EVENT_RDY_RECV_RELEASE, &outMsg, true);
    }
    return true;
}
