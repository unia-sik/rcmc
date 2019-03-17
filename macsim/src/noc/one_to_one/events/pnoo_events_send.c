#include "pnoo_events_send.h"

#include "pnoo_timing.h"
#include "share.h"
#include "pnoo.h"

extern node_t* nodes[];

void pnoo_events_send_init(pnoo_event_queue_t* queue, pnoo_context_t* context)
{
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_SEND_BUFFER_REMOVE, (pnoo_event_queue_callback_t) pnoo_events_send_remove, context);
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_SEND_BUFFER_DECREMENT, (pnoo_event_queue_callback_t) pnoo_events_send_decrement, context);    
}

bool pnoo_events_send_remove(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_context_t* contextDest = (pnoo_context_t*) nodes[msg->flit.dest]->noc_context;
    
    if ((!PNOO_ENABLE_CONGESTION_CONTROL || pnoo_bit_array_check(&context->commitBitArray, msg->flit.dest)) && num_event == 1) {
        pnoo_context_add_timed_event(context, clk, PNOO_EVENT_SEND_BUFFER_DECREMENT, msg, false);
        pnoo_context_add_timed_event(contextDest, clk, PNOO_EVENT_RECV_BUFFER_INSERT, msg, false);
        pnoo_context_add_timed_event(contextDest, clk, PNOO_EVENT_RECV_BUFFER_INCREMENT, msg, false);        
    } else {
        pnoo_event_queue_item_t* lastEvent = pnoo_event_queue_search_last(&context->eventQueue, PNOO_EVENT_SEND_BUFFER_REMOVE);

        if (num_event == 1 || lastEvent == NULL || lastEvent->clk == clk) {
            pnoo_context_add_timed_event(context, clk, PNOO_EVENT_SEND_BUFFER_REMOVE, msg, true);
        } else {
            pnoo_event_queue_insert(&context->eventQueue, lastEvent->clk, PNOO_EVENT_SEND_BUFFER_REMOVE, msg);
        }
    }

    return true;
}

bool pnoo_events_send_decrement(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    context->num_sendbuffer_entries--;
    return true;
}
