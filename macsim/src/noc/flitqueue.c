/**
 * Infinite queues for flit containers
 *
 * MacSim project
 */
#include "flitqueue.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Add a message to the end of a queue
bool fc_enqueue(fc_queue_t *queue, flit_container2_t fc)
{
    fc_queue_entry_t *n = malloc(sizeof(fc_queue_entry_t));
    if (!n) return false; // Out of memory
    n->next = 0;
    n->fc = fc;
    if (queue->tail) {
        queue->tail->next = n;
        queue->tail = n;
    } else
        queue->tail = queue->head = n;
    queue->count++;
    return true;
}

// Remove a message from the head of a queue
bool fc_dequeue(fc_queue_t *queue, flit_container2_t *fc)
{
    fc_queue_entry_t *h = queue->head;
    if (!h) return false; // no entry
    *fc = h->fc;
    queue->head = h->next;
    if (!h->next) queue->tail = 0; // only one entry
    free(h);
    queue->count--;
    return true;
}

// Search for the first message with matching rank and remove it from the queue,
// even if it is in the middle
bool fc_dequeue_src(fc_queue_t *queue, rank_t src, flit_container2_t *fc)
{
    fc_queue_entry_t *p=0, *h=queue->head;
    while (h) {
        if (h->fc.src == src) {
            *fc = h->fc;
            if (p) p->next = h->next;
            else queue->head = h->next;
            if (!h->next) queue->tail = p;
            free(h);
            queue->count--;
            return true;
        }
        p = h;
        h = h->next;
    }
    return false;
}

// Search for the first message with matching rank and return its position
long int fc_find_src(fc_queue_t *queue, rank_t src)
{
    fc_queue_entry_t *h=queue->head;
    long int i=0;
    while (h) {
        if (h->fc.src == src) return i;
        i++;
        h = h->next;
    }
    return -1;
}



// Add a queue to the end of another queue
void fc_cat_queue(fc_queue_t *base, fc_queue_t *add)
{
    if (base->tail) {
        base->tail->next = add->head;
        if (add->tail) base->tail = add->tail;
        base->count += add->count;
    } else {
        base->head = add->head;
        base->tail = add->tail;
        base->count = add->count;
    }
}

// Init queue
void fc_init_queue(fc_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

// Destroy queue
void fc_destroy_queue(fc_queue_t *queue)
{
    fc_queue_entry_t *p=queue->head;
    while (p) {
        fc_queue_entry_t *n = p->next;
        free(p);
        p = n;
    }
}



void fc_print_queue(fc_queue_t *queue)
{
    fc_queue_entry_t *e = queue->head;
    while (e) {
        printf("<%lx>", e->fc.src);
        e = e->next;
    }
    printf("|");
}

