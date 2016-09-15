/**
 * Infinite queues for flit containers
 *
 * MacSim project
 */
#ifndef _FLITQUEUE_H
#define _FLITQUEUE_H

#include "share.h"
#include "flitfifo.h"


typedef struct fc_queue_entry_s {
    struct fc_queue_entry_s     *next;
    flit_container2_t           fc;
} fc_queue_entry_t;

typedef struct {
    fc_queue_entry_t		*head;
    fc_queue_entry_t		*tail;
    uint32_t			count;
} fc_queue_t;


// Add a message to the end of a queue
bool fc_enqueue(fc_queue_t *queue, flit_container2_t fc);

// Remove a message from the head of a queue
bool fc_dequeue(fc_queue_t *queue, flit_container2_t *fc);

// Search for the first message with matching rank and return its position
long int fc_find_src(fc_queue_t *queue, rank_t rank);

// Search for the first message with matching rank and remove it from the queue,
// even if it is in the middle
bool fc_dequeue_src(fc_queue_t *queue, rank_t rank, flit_container2_t *fc);

// Add a queue to the end of another queue
void fc_cat_queue(fc_queue_t *base, fc_queue_t *add);

// Init queue
void fc_init_queue(fc_queue_t *queue);

// Destroy queue
void fc_destroy_queue(fc_queue_t *queue);

// Print queue
void fc_print_queue(fc_queue_t *queue);







#endif
