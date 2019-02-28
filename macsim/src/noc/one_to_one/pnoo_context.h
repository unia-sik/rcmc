#pragma once

#include "share.h"
#include "pnoo_recv_buffer.h"
#include "pnoo_bit_array.h"
#include "pnoo_event_queue.h"
#include "pnoo_events.h"

#define PNOO_CONTEXT_SENDBUFFER_SIZE 32
#define PNOO_CONTEXT_RECVBUFFER_SIZE 32

typedef struct {
    uint64_t num_sendbuffer_entries;
    uint64_t num_recvbuffer_entries;
    
    rank_t barrierMin;
    rank_t barrierMax;
    rank_t barrierCount;
    rank_t barrierCurrent;
    bool barrierEnable;
    cycle_t barrierLastAccess;
        
    pnoo_recv_buffer_t recvBuffer;
    pnoo_bit_array_t bitArray;
    pnoo_bit_array_t commitBitArray;
    pnoo_event_queue_t eventQueue;
    
    rank_t srdyDest;
} pnoo_context_t;

void pnoo_context_init(pnoo_context_t* context);
void pnoo_context_destroy(pnoo_context_t* context);
void pnoo_context_add_timed_event(pnoo_context_t* context, const cycle_t clk, const pnoo_events_t event, const pnoo_msg_t* msg, const bool delayed);

