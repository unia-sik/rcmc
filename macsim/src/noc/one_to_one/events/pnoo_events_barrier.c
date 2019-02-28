#include "pnoo_events_barrier.h"

#include "node.h"
#include "pnoo_timing.h"


extern node_t* nodes[];

void pnoo_events_barrier_init(pnoo_event_queue_t* queue, pnoo_context_t* context)
{
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_BARRIER_SET, (pnoo_event_queue_callback_t)pnoo_events_barrier_callback, context);
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_BARRIER_ENABLE, (pnoo_event_queue_callback_t)pnoo_events_barrier_enable_callback, context);    
    pnoo_event_queue_register_callback(queue, PNOO_EVENT_BARRIER_CLEAR, (pnoo_event_queue_callback_t)pnoo_events_barrier_clear_callback, context);    
}

bool pnoo_events_barrier_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    rank_t x = (msg->flit.src % conf_noc_width);
    rank_t y = (msg->flit.src / conf_noc_width);
    
    rank_t minX = context->barrierMin & 0xFFFF;
    rank_t minY = (context->barrierMin & 0xFFFF0000) >> 16;
    rank_t maxX = context->barrierMax & 0xFFFF;
    rank_t maxY = (context->barrierMax & 0xFFFF0000) >> 16;
  
    if (minX <= x && x <= maxX && minY <= y && y <= maxY && context->barrierEnable) {
        context->barrierCurrent++;
        context->barrierLastAccess = clk;
    }
    
    return true;
}

bool pnoo_events_barrier_enable_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{
    pnoo_context_add_timed_event(context, clk, PNOO_EVENT_BARRIER_CLEAR, msg, false);    
        
    for (int x = (context->barrierMin & 0xFFFF); x <= (context->barrierMax & 0xFFFF); x++) {
        for (int y = ((context->barrierMin & 0xFFFF0000) >> 16); y <= ((context->barrierMax & 0xFFFF0000) >> 16); y++) {
            rank_t rank = y * conf_noc_width + x;
            
            if (msg->flit.dest != rank) {
                pnoo_context_t* contextI = nodes[rank]->noc_context;
                pnoo_msg_t msgI = pnoo_msg_init(msg->flit.dest, rank);
                pnoo_context_add_timed_event(contextI, clk, PNOO_EVENT_BARRIER_SET, &msgI, true);    
            }
        }    
    }
    
    context->barrierEnable = true;
    context->barrierCurrent = 1;
    
    return true;
}

bool pnoo_events_barrier_clear_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk)
{    
    pnoo_event_queue_item_t* nextSet = pnoo_event_queue_search(&context->eventQueue, PNOO_EVENT_BARRIER_SET);
    
    if (nextSet != NULL && nextSet->clk == clk) {
        pnoo_event_queue_insert(&context->eventQueue, clk, PNOO_EVENT_BARRIER_CLEAR, msg);
        return true;
    }    
    
    if (context->barrierEnable && context->barrierCount != context->barrierCurrent) {
        pnoo_context_add_timed_event(context, clk, PNOO_EVENT_BARRIER_CLEAR, msg, true);
            
        for (int x = (context->barrierMin & 0xFFFF); x <= (context->barrierMax & 0xFFFF); x++) {
            for (int y = ((context->barrierMin & 0xFFFF0000) >> 16); y <= ((context->barrierMax & 0xFFFF0000) >> 16); y++) {
                rank_t rank = y * conf_noc_width + x;
                
                if (msg->flit.dest != rank) {
                    pnoo_context_t* contextI = nodes[rank]->noc_context;
                    pnoo_msg_t msgI = pnoo_msg_init(msg->flit.dest, rank);
                    pnoo_context_add_timed_event(contextI, clk, PNOO_EVENT_BARRIER_SET, &msgI, true);    
                }
            }    
        }        
        
        context->barrierCurrent = 1;    
    }     
    
    return true;
}
