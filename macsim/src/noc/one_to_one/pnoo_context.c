#include "pnoo_context.h"
#include "events/pnoo_events_send.h"
#include "events/pnoo_events_recv.h"
#include "events/pnoo_events_rdy.h"
#include "events/pnoo_events_barrier.h"
#include "pnoo_timing.h"

void pnoo_context_init(pnoo_context_t* context)
{
    pnoo_recv_buffer_init(&context->recvBuffer, PNOO_CONTEXT_RECVBUFFER_SIZE);
    pnoo_bit_array_init(&context->bitArray);
    pnoo_bit_array_init(&context->commitBitArray);

    context->num_sendbuffer_entries = 0;
    context->num_recvbuffer_entries = 0;
    context->srdyDest = -1;

    context->barrierEnable = false;
    context->barrierLastAccess = 0;
    
    pnoo_event_queue_init(&context->eventQueue);
    pnoo_events_send_init(&context->eventQueue, context);
    pnoo_events_recv_init(&context->eventQueue, context);
    pnoo_events_rdy_init(&context->eventQueue, context);
    pnoo_events_barrier_init(&context->eventQueue, context);
}

void pnoo_context_destroy(pnoo_context_t* context)
{
    pnoo_recv_buffer_destroy(&context->recvBuffer);
}

void pnoo_context_add_timed_event(pnoo_context_t* context, const cycle_t clk, const pnoo_events_t event, const pnoo_msg_t* msg, const bool delayed)
{
    pnoo_event_queue_insert(
        &context->eventQueue,
        pnoo_timing_calc_next_event_time(
            clk,
            event,
            msg->flit.src,
            msg->flit.dest,
            delayed
        ),
        event,
        msg
    );
}


