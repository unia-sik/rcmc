#pragma once

#include "share.h"
#include "pnoo_context.h"
#include "pnoo_msg.h"
#include "pnoo_event_queue.h"

void pnoo_events_rdy_init(pnoo_event_queue_t* queue, pnoo_context_t* context);
bool pnoo_events_rdy_recv_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk);
bool pnoo_events_rdy_recv_commit_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk);
bool pnoo_events_rdy_recv_release_callback(pnoo_context_t* context, const pnoo_msg_t* msg, const int num_event, const cycle_t clk);

