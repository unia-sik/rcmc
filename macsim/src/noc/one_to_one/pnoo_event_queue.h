#pragma once

#include "share.h"
#include "pnoo_msg.h"
#include "pnoo_events.h"

typedef struct pnoo_event_queue_item_s {
    uint64_t clk;
    pnoo_events_t event;
    pnoo_msg_t data;
    struct pnoo_event_queue_item_s* next;
    
    struct pnoo_event_queue_item_s* left;
    struct pnoo_event_queue_item_s* bot;
} pnoo_event_queue_item_t;

typedef bool(*pnoo_event_queue_callback_t)(void*, const pnoo_msg_t*, const int, const cycle_t);
#define fast_track_size 1000
typedef struct {
    pnoo_event_queue_callback_t callbacks[pnoo_events_count];
    void* callbackArgs[pnoo_events_count];
    pnoo_event_queue_item_t* lastItem[pnoo_events_count];
    pnoo_event_queue_item_t* head;
    pnoo_event_queue_item_t* tail;
    
    pnoo_event_queue_item_t* graveyard;
    
    pnoo_event_queue_item_t* root;
    
    pnoo_event_queue_item_t* fastTrack[fast_track_size];
} pnoo_event_queue_t;

void pnoo_event_queue_init(pnoo_event_queue_t* queue);
void pnoo_event_queue_register_callback(pnoo_event_queue_t* queue, const pnoo_events_t event, const pnoo_event_queue_callback_t callback, void* arg);
void pnoo_event_queue_insert(pnoo_event_queue_t* queue, const uint64_t clk, const pnoo_events_t event, const pnoo_msg_t* data);
void pnoo_event_queue_execute(pnoo_event_queue_t* queue, const uint64_t clk);

bool pnoo_event_queue_contains(const pnoo_event_queue_t* queue, const pnoo_events_t event);
pnoo_event_queue_item_t* pnoo_event_queue_search(const pnoo_event_queue_t* queue, const pnoo_events_t event);
pnoo_event_queue_item_t* pnoo_event_queue_search_last(const pnoo_event_queue_t* queue, const pnoo_events_t event);

void pnoo_event_queue_print(const pnoo_event_queue_t* queue);
