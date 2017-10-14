/**
 * traffic_context.h
 * Core context for synthetic traffic patterns
 *
 * MacSim project
 */
#ifndef _TRAFFIC_CONTEXT_H
#define _TRAFFIC_CONTEXT_H

#include "share.h"

typedef struct
{
    uint_fast16_t       type;
    uint_fast16_t       nodebits; // 2^nodebits == conf_max_rank
    flit_queue_entry_t  *injqueue_head;
    flit_queue_entry_t  *injqueue_tail;

    cycle_t             stat_flit_count;
    cycle_t             stat_total_latency;
} traffic_context_t;

#endif
