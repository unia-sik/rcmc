/** 
 * $Id: wwabstract.h 509 2013-04-08 22:17:56Z mischejo $
 * Queues for messages
 *
 * McSim project
 */
#ifndef _MSGQUEUE_H
#define _MSGQUEUE_H

//#include "config.h"
#define MAX_MSG_LEN 256
#include "share.h"

#define FIXEDLAT_TRANSPORT_DELAY	3	// #Rounds a msg is within the interconnect


typedef struct msg_queue_entry_s {
    struct msg_queue_entry_s	*next;
    rank_t 			rank;
    uint32_min_t		len;
    char			content[MAX_MSG_LEN];
    // Do not add any field variables after this point!
    // When allocatin memory for a msg_queue_entry, not memory for the whole
    // structure is allocated, but only enough bytes to hold len chars
    // within content. content[len+1] and all later bytes can already belong
    // to another variable.
} msg_queue_entry_t;

typedef struct {
    msg_queue_entry_t		*head;
    msg_queue_entry_t		*tail;
    uint32_t			count;
} msg_queue_t;


// Add a message to the end of a queue
bool msg_enqueue(msg_queue_t *queue, uint8_t *buf, uint32_min_t len, rank_t rank);

// Remove a message from the head of a queue
bool msg_dequeue(msg_queue_t *queue, uint8_t *buf, uint32_min_t *len, rank_t *rank);

// Search for the first message with matching rank and remove it from the queue,
// even if it is in the middle
bool msg_dequeue_rank(msg_queue_t *queue, uint8_t *buf, uint32_min_t *len, rank_t rank);

// Add a queue to the end of another queue
void msg_cat_queue(msg_queue_t *base, msg_queue_t *add);

// Init queue
void msg_init_queue(msg_queue_t *queue);

// Print queue
void msg_print_queue(msg_queue_t *queue);



#endif // _MSGQUEUE_H
