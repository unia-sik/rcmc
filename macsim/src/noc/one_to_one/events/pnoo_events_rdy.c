#include "pnoo_events_rdy.h"

#include "node.h"
#include "pnoo_timing.h"

extern node_t* nodes[];

void pnoo_events_rdy_init(pnoo_event_queue_t* queue, pnoo_context_t* context)
{
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_RDY_RECV, (pnoo_event_queue_callback_t)pnoo_events_rdy_recv_callback, context);
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_RDY_RECV_COMMIT, (pnoo_event_queue_callback_t)pnoo_events_rdy_recv_commit_callback, context);
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_RDY_RECV_RELEASE, (pnoo_event_queue_callback_t)pnoo_events_rdy_recv_release_callback, context);
}

bool pnoo_events_rdy_recv_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_bit_array_set(&context->bitArray, msg->flit.src);
    return true;
}

bool pnoo_events_rdy_recv_commit_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_bit_array_set(&context->commitBitArray, msg->flit.src);
    return true;
}

bool pnoo_events_rdy_recv_release_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_bit_array_unset(&context->commitBitArray, msg->flit.src);
    return true;
}
