#pragma once

#include "share.h"
#include "node.h"

#include "pnoo_events.h"

cycle_t pnoo_timing_calc_access_time(const node_t *node);
cycle_t pnoo_timing_calc_next_event_time(const cycle_t clk, const pnoo_events_t event, const rank_t src, const rank_t dest, const bool delayed);
