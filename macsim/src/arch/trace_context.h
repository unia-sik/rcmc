/**
 * trace_context.h
 * Core context for netrace simulation
 *
 * MacSim project
 */
#ifndef _TRACE_CONTEXT_H
#define _TRACE_CONTEXT_H

#include "share.h"

typedef struct
{
    uint_fast16_t       type;
    uint_fast16_t       nodebits; // 2^nodebits == conf_max_rank
    flit_queue_entry_t  *injqueue_head;
    flit_queue_entry_t  *injqueue_tail;

    cycle_t             behind;         // how many cycles behind the trace
    unsigned            pending_len;    // how many flits are pending
    rank_t              pending_dest;   // to which node
    flit_t              pending_flit;   // payload = index in trace_packets
} netrace_context_t;

#endif
