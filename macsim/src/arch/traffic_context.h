/**
 * traffic_context.h
 * Core context for synthetic traffic patterns
 *
 * MacSim project
 */
#ifndef _TRAFFIC_CONTEXT_H
#define _TRAFFIC_CONTEXT_H

#include "share.h"

typedef struct injqueue_entry_s
{
    struct injqueue_entry_s     *next;
    rank_t                      dest;
    flit_t                      flit;
} injqueue_entry_t;

typedef struct
{
    uint_fast16_t               type;
    uint_fast16_t               nodebits; // 2^nodebits == conf_max_rank
    struct injqueue_entry_s     *injqueue_head;
    struct injqueue_entry_s     *injqueue_tail;

    cycle_t                     stat_flit_count;
    cycle_t                     stat_total_latency;
} traffic_context_t;

#endif
