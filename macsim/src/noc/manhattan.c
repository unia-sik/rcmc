/** 
 * Latency depending on the Manhattan distance, ignoring collisions
 *
 * McSim project
 */
#include "manhattan.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define CONF_CYCLES_PER_HOP 3

//#define DEBUG printf
#define DEBUG(...)


extern node_t *nodes[];

typedef struct {
    fc_queue_t intransit;
    fc_queue_t arrived;
} manhattan_context_t;



// Send a flit to a core
bool manhattan_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    DEBUG("S %lu->%lu [%lx]\n", node->rank, dest, flit);

    long int dx = labs(X_FROM_RANK(node->rank) - X_FROM_RANK(dest));
    long int dy = labs(Y_FROM_RANK(node->rank) - Y_FROM_RANK(dest));
    cycle_t latency = CONF_CYCLES_PER_HOP*(dx+dy);

    assert (latency>0 && "Same source and dest node");

    flit_container2_t fc;
    fc.src = node->rank;
    fc.dest = latency; // misuse dest for latency counting
    fc.flit = flit;

    manhattan_context_t *dest_context = nodes[dest]->noc_context;
    if (!fc_enqueue(&dest_context->intransit, fc))
        fatal("Out of memory");
    return 1;
}


// Receive a flit from a specified core
bool manhattan_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container2_t fc;
    bool r = fc_dequeue_src(&((manhattan_context_t *)node->noc_context)->
        arrived,  src, &fc);
    *flit = fc.flit;
    if (r) DEBUG("R %lu->%lu [%lx]\n", src, node->rank, *flit);
    return r;
}


bool manhattan_sender_ready(node_t *node)
{
    return 1;
}


bool manhattan_probe_rank(node_t *node, rank_t src)
{
    manhattan_context_t *context = node->noc_context;
    long int pos = fc_find_src(&context->arrived, src);
    return pos > 0;
}


rank_t manhattan_probe_any(node_t *node)
{
    manhattan_context_t *context = node->noc_context;
    return (context->arrived.head==0)
        ? (rank_t)-1
        : context->arrived.head->fc.src;
}


void manhattan_route_one_cycle(node_t *node)
{
    manhattan_context_t *context = node->noc_context;
    fc_queue_t *intransit = &context->intransit;
    fc_queue_entry_t *h = intransit->head;
    fc_queue_entry_t *p = 0;

    // Go through intransit list and decrement latency (stored in dest field)
    // If latency==0, remove from intransit and add to arrived list
    while (h) {
        cycle_t latency = h->fc.dest;
        latency--;
        if (latency==0) {

            // remove from intransit
            if (p) p->next = h->next;
            else intransit->head = h->next;
            if (!h->next) intransit->tail = p;
            intransit->count--;

            // add to arrived
            fc_queue_t *arrived = &context->arrived;
            h->next = 0;
            if (arrived->tail) {
                arrived->tail->next = h;
                arrived->tail = h;
            } else
                arrived->tail = arrived->head = h;
            arrived->count++;
        } else {
            h->fc.dest = latency;
        }
        p = h;
        h = h->next;
    }
}


// Init the interconnection simulator
void manhattan_init(node_t *node)
{
    // set function pointers
    node->noc_send_flit          = manhattan_send_flit;
    node->noc_recv_flit          = manhattan_recv_flit;
    node->noc_sender_ready       = manhattan_sender_ready;
    node->noc_probe_rank         = manhattan_probe_rank;
    node->noc_probe_any          = manhattan_probe_any;
    node->noc_route_one_cycle    = manhattan_route_one_cycle;
    manhattan_context_t *context = fatal_malloc(sizeof(manhattan_context_t));
    node->noc_context = context;

    fc_init_queue(&context->intransit);
    fc_init_queue(&context->arrived);
}

void manhattan_destroy(node_t *node)
{
    manhattan_context_t *context = node->noc_context;
    fc_destroy_queue(&context->intransit);
    fc_destroy_queue(&context->arrived);
    free(node->noc_context);
}



static void print_queue(fc_queue_t *queue)
{
    fc_queue_entry_t *e = queue->head;
    while (e) {
        printf("<%ld>", e->fc.dest);
        e = e->next;
    }
    printf("|");
}


void manhattan_print_context(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    for (r=0; r<max_rank; r++) {
        manhattan_context_t *self = nodes[r]->noc_context;
        printf("$%ld: ", r);
        print_queue(&self->intransit);
        print_queue(&self->arrived);
        printf("\n");
    }
}
