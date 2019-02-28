#include "pnoo_event_queue.h"
#include <stdlib.h>


void pnoo_event_queue_init(pnoo_event_queue_t* queue)
{
    for (int i = 0; i < pnoo_events_count; i++) {
        queue->callbacks[i] = NULL;
        queue->lastItem[i] = NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->root = NULL;
    queue->graveyard = NULL;

    for (int i = 0; i < fast_track_size; i++) {
        queue->fastTrack[i] = NULL;
    }
}

pnoo_event_queue_item_t* pnoo_event_queue_get_fasttrack_start(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* item)
{
    size_t position = (item->clk / 1) % fast_track_size;

    for (int i = 0; i < fast_track_size; i++) {
        int index = (position - i);
        
        if (index < 0) {
            index += fast_track_size;
        }

        if (queue->fastTrack[index] != NULL) {
            if (queue->fastTrack[index]->clk <= item->clk) {

//                 for (int j = 1; j < i; j++) {
//                     int indexSet = (position + fast_track_size - j) % fast_track_size;
//                     queue->fastTrack[indexSet] = queue->fastTrack[index];
//                 }


                return queue->fastTrack[index];
            }
        }
    }



    return queue->head;
}

void pnoo_event_queue_insert_fasttrack(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* item)
{
    int position = (item->clk / 1) % fast_track_size;
    queue->fastTrack[position] = item;

//     for (int i = 1; i < fast_track_size; i++) {
//         int index = (position + fast_track_size - i) % fast_track_size;
//         if (queue->fastTrack[index] != NULL || queue->fastTrack[index]->clk <= item->clk) {
//             return;
//         }
//         queue->fastTrack[index] = item;
//     }
}

void pnoo_event_queue_remove_fasttrack(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* item)
{
    if (queue->fastTrack[(item->clk / 1) % fast_track_size] == item) {
        queue->fastTrack[(item->clk / 1) % fast_track_size] = NULL;
    }

//     int position = (item->clk / 1) % fast_track_size;
//     for (int i = 1; i < fast_track_size; i++) {
//         int index = (position + fast_track_size - i) % fast_track_size;
//         if (queue->fastTrack[index] != item) {
//             return;
//         }
//         queue->fastTrack[index] = NULL;
//     }
}

void pnoo_event_queue_register_callback(pnoo_event_queue_t* queue, const pnoo_events_t event, const pnoo_event_queue_callback_t callback, void* arg)
{
    queue->callbacks[event] = callback;
    queue->callbackArgs[event] = arg;
}

void pnoo_event_queue_insert_item(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* item)
{
    pnoo_event_queue_item_t* current;
    pnoo_event_queue_item_t* old = NULL;

//     if (queue->tail != NULL && queue->tail->clk <= item->clk) {
//         current = queue->tail;
//     } else {
        current = pnoo_event_queue_get_fasttrack_start(queue, item);
//     }


    int n  = 0;
    int x;

//     if (queue->tail != NULL) {
//         x = queue->tail->clk;
//     } else {
//         x = -1;
//     }

//     current = queue->head;
//     printf("###%ld  %ld\n", queue->head == current, x);

    while (current != NULL && current->clk <= item->clk) {
//         printf("-->%ld %ld\n", current->clk, item->clk);

        old = current;
        current = current->next;
        n++;


    }
    if (n != 1) {
//         printf(">>%ld\n", n);
    }
    item->next = current;

    if (old == NULL) {
        queue->head = item;
    } else {
        old->next = item;
    }

//     if (queue->tail == NULL || queue->tail->clk <= item->clk) {
//         queue->tail = item;
//     }

    if (queue->lastItem[item->event] == NULL || queue->lastItem[item->event]->clk < item->clk) {
        queue->lastItem[item->event] = item;
    }

    pnoo_event_queue_insert_fasttrack(queue, item);
}

void pnoo_event_queue_insert(pnoo_event_queue_t* queue, const uint64_t clk, const pnoo_events_t event, const pnoo_msg_t* data)
{
    pnoo_event_queue_item_t* newObj;
    
    if (queue->graveyard != NULL) {
        newObj = queue->graveyard;
        queue->graveyard = queue->graveyard->next;
//         newObj->next = NULL;
    } else {
        newObj = (pnoo_event_queue_item_t*) malloc(sizeof(pnoo_event_queue_item_t));
    }
    newObj->clk = clk;
    newObj->event = event;
    newObj->data = *data;

    pnoo_event_queue_insert_item(queue, newObj);
}

bool pnoo_event_queue_execute_callback(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* item, int* num_event)
{
    assert(queue->callbacks[item->event] != NULL);
    assert(queue->callbackArgs[item->event] != NULL);

    num_event[item->event]++;
    return queue->callbacks[item->event](queue->callbackArgs[item->event], &item->data, num_event[item->event], item->clk);
}

pnoo_event_queue_item_t* pnoo_event_queue_remove(pnoo_event_queue_t* queue, pnoo_event_queue_item_t* prev, pnoo_event_queue_item_t* current)
{
    if (queue->lastItem[current->event] == current) {
        queue->lastItem[current->event] = NULL;
    }

    if (queue->tail == current) {
        queue->tail = NULL;
    }

    if (prev == NULL) {
        if (queue->head == current) {
            queue->head = current->next;
            pnoo_event_queue_remove_fasttrack(queue, current);
            
            current->next = queue->graveyard;
            queue->graveyard = current;
//             free(current);
            return queue->head;
        } else {
            prev = queue->head;
        }
    }

    if (prev->next == current) {
        prev->next = current->next;
        pnoo_event_queue_remove_fasttrack(queue, current);
        
        current->next = queue->graveyard;
        queue->graveyard = current;
//         free(current);
        return prev->next;
    }

    while (prev->next != current) {
        prev = prev->next;
    }

    prev->next = current->next;
    pnoo_event_queue_remove_fasttrack(queue, current);

    current->next = queue->graveyard;
    queue->graveyard = current;
//     free(current);
    return prev->next;
}

void pnoo_event_queue_execute(pnoo_event_queue_t* queue, const uint64_t clk)
{
    pnoo_event_queue_item_t* current = queue->head;
    pnoo_event_queue_item_t* old = NULL;
    int num_event[pnoo_events_count];

    for (int i = 0; i < pnoo_events_count; i++) {
        num_event[i] = 0;
    }

    while (current != NULL && current->clk <= clk) {
        if (pnoo_event_queue_execute_callback(queue, current, num_event)) {
            current = pnoo_event_queue_remove(queue, old, current);
        } else {
            old = current;
            current = current->next;
        }
    }
}

bool pnoo_event_queue_contains(const pnoo_event_queue_t* queue, const pnoo_events_t event)
{
    return pnoo_event_queue_search(queue, event) != NULL;
}

pnoo_event_queue_item_t* pnoo_event_queue_search(const pnoo_event_queue_t* queue, const pnoo_events_t event)
{
    for (pnoo_event_queue_item_t* current = queue->head; current != NULL; current = current->next) {
        if (current->event == event) {
            return current;
        }
    }

    return NULL;
}

pnoo_event_queue_item_t* pnoo_event_queue_search_last(const pnoo_event_queue_t* queue,  const pnoo_events_t event)
{
    return queue->lastItem[event];

    pnoo_event_queue_item_t* result = NULL;

    for (pnoo_event_queue_item_t* current = queue->head; current != NULL; current = current->next) {
        if (current->event == event) {
            result = current;
        }
    }

    return result;
}

void pnoo_event_queue_print(const pnoo_event_queue_t* queue)
{
    for (pnoo_event_queue_item_t* current = queue->head; current != NULL; current = current->next) {
        uint64_t flit = * ((uint64_t*) &current->data.flit.payload);
        printf("cycle %ld: %ld ----(%s, 0x%08lx)----> %ld\n", current->clk, current->data.flit.src, pnoo_events_to_string(current->event), flit, current->data.flit.dest);
    }
}


