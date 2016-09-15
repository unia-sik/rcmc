/** 
 * $Id: wwabstract.c 509 2013-04-08 22:17:56Z mischejo $
 * Fixed latency abstraction of the NoC
 *
 * McSim project
 */
#include "msgqueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Add a message to the end of a queue
bool msg_enqueue(msg_queue_t *queue, uint8_t *buf, uint32_min_t len, rank_t rank)
{
    msg_queue_entry_t *n = malloc(sizeof(msg_queue_entry_t)-MAX_MSG_LEN+len);
    if (!n) return 0; // Out of memory
    n->next = 0;
    n->rank = rank;
    n->len = len;
    memcpy(n->content, buf, len);
    if (queue->tail)
    {
	queue->tail->next = n;
	queue->tail = n;
    }
    else
	queue->tail = queue->head = n;
    queue->count++;
    return 1;
}    

// Remove a message from the head of a queue
bool msg_dequeue(msg_queue_t *queue, uint8_t *buf, uint32_min_t *len, rank_t *rank)
{
    msg_queue_entry_t *h = queue->head;
    if (!h) return 0; // no entry
    *rank = h->rank;
    *len = h->len;
    memcpy(buf, h->content, h->len);
    queue->head = h->next;
    if (!h->next) // only one entry
	queue->tail = 0;
    free(h);
    queue->count--;
    return 1;
}

// Search for the first message with matching rank and remove it from the queue,
// even if it is in the middle
bool msg_dequeue_rank(msg_queue_t *queue, uint8_t *buf, uint32_min_t *len, rank_t rank)
{
    msg_queue_entry_t *p=0, *h=queue->head;
    while (h)
    {
	if (h->rank == rank)
	{
	    *len = h->len;
	    memcpy(buf, h->content, h->len);
	    if (p)
		p->next = h->next;
	    else
		queue->head = h->next;
	    if (!h->next)
	        queue->tail = p;
	    free(h);
	    queue->count--;
	    return 1;
	}
	p = h;
	h = h->next;
    }
    return 0;
}

// Add a queue to the end of another queue
void msg_cat_queue(msg_queue_t *base, msg_queue_t *add)
{
    if (base->tail)
    {
	base->tail->next = add->head;
	if (add->tail)
	    base->tail = add->tail;
	base->count += add->count;
    }
    else
    {
	base->head = add->head;
	base->tail = add->tail;
	base->count = add->count;
    }
}

// Init queue
void msg_init_queue(msg_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

// Print queue
void msg_print_queue(msg_queue_t *queue)
{
    msg_queue_entry_t *e = queue->head;
    while (e)
    {
	printf("<%s>", e->content);
	e = e->next;
    }
    printf("|");
}

