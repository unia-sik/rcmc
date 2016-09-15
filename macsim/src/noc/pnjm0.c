/*
 * pnj0.c
 * Original PaterNoster implementation from boooksim simulation
 *
 * RC/MC project
 *
 * TODO:
 * - don't use conf_noc_width
 * - rename XXX_init() to XXX_init_router() and move memory allocation to macsim main
 */

#include "pnjm0.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


#define DEBUG(...)
//#define DEBUG printf



#define conf_send_fifo_size     8
#define conf_recv_fifo_size     (2*conf_max_rank)
#define conf_corner_fifo_size   8


unsigned long stat_stall_east;
unsigned long stat_stall_north;
unsigned long stat_stall_x;
unsigned long stat_stall_y;
unsigned long stat_stall_west;
unsigned long stat_stall_south;
unsigned long stat_deflect_west;
unsigned long stat_deflect_south;





bool pnjm0_send_flit(node_t *node, rank_t dest, flit_t flit)
{
//flitfifo_print(&((pnjm0_context_t *)node->noc_context)->send_fifo);

    flit_container2_t fc;
    fc.src = node->rank;
    fc.dest = dest;
    fc.flit = flit;
    bool r = flitfifo_insert(&((pnjm0_context_t *)node->noc_context)->send_fifo, &fc);
    if (r) DEBUG("S %lu->%lu(%lx)\n", node->rank, dest, flit);
    return r;
}

bool pnjm0_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container2_t fc;

if (node->rank==4) {
// flitfifo_print(&((pnjm0_context_t *)node->noc_context)->recv_fifo);
// printf(" RECV %lu->%lu\n", src, node->rank);
}

    if (flitfifo_remove_by_rank(&((pnjm0_context_t *)node->noc_context)->recv_fifo, src, &fc)) {
//flitfifo_print(&((pnjm0_context_t *)node->noc_context)->recv_fifo); //printf("\n");
        DEBUG("R %lu->%lu(%lx)\n", src, node->rank, fc.flit);
        *flit = fc.flit;
        return true;
    }
    return false;
}

bool pnjm0_sender_ready(node_t *node)
{
    return !flitfifo_full(&((pnjm0_context_t *)node->noc_context)->send_fifo);
}

bool pnjm0_probe_rank(node_t *node, rank_t src)
{
    return flitfifo_find_rank(&((pnjm0_context_t *)node->noc_context)->recv_fifo, src) >= 0;
}

rank_t pnjm0_probe_any(node_t *node)
{
    return flitfifo_first_rank(&((pnjm0_context_t *)node->noc_context)->recv_fifo);
}







static void router_one_cycle(pnjm0_context_t *r)
{
#define EMPTY 0
#define INJECT_FROM_LOCAL 1
#define FORWARD_FROM_WEST 2
#define DEFLECT_FROM_WEST 3

#define FROM_CORNER 1
#define FORWARD_FROM_SOUTH 2
#define DEFLECT_FROM_SOUTH 3

#define EJECT_FROM_SOUTH 1
#define EJECT_FROM_WEST 2

#define TURN_FROM_WEST 2

    unsigned east = EMPTY;
    unsigned north = EMPTY;
    unsigned local = EMPTY;
    unsigned corner = EMPTY;


/*
    if (r->in_north_request) {
        r->request_from_north = true;
    }
    if (r->in_east_request) {
        r->request_from_east = true;
    }
*/



    // stall counters

    if (r->stall_south!=0) r->stall_south--; 
        // don't eject for 1 round after an ejection was rejected
    if (r->stall_north!=0) r->stall_north--; 
    else if (r->in_south_deflected) r->stall_north = conf_noc_height;
        // don't inject for 1 round after any deflected flit came by
    if (r->in_south_valid && Y_FROM_RANK(r->in_south_fc.src) == r->y)
        r->stall_y = conf_noc_height;
    else if (r->stall_y!=0) r->stall_y--;
        // don't inject as long as there are older flits in the ring that took
        // an extra round and were injected from here

    if (r->stall_west!=0) r->stall_west--;
    if (r->stall_east!=0) r->stall_east--;
    else if (r->in_west_deflected) r->stall_east = conf_noc_width;
    if (r->in_west_valid && X_FROM_RANK(r->in_west_fc.src) == r->x)
        r->stall_x = conf_noc_width;
    else if (r->stall_x!=0) r->stall_x--;


    // south to north decisions

    unsigned y_dest = Y_FROM_RANK(r->in_south_fc.dest);
    if (r->in_south_valid && ((y_dest!=r->y) || r->stall_south)) {
        if (y_dest==r->y) stat_stall_south++;
        north = FORWARD_FROM_SOUTH; // forward south to north
    } else {
        if (r->in_south_valid && y_dest==r->y) {
            if (flitfifo_full(&r->recv_fifo)) {
                north = DEFLECT_FROM_SOUTH; // forward south to north, mark as deflected
                stat_deflect_south++;
            } else {
                local = EJECT_FROM_SOUTH; // eject south to local
            }
        }
    }

    if ((north==EMPTY)
        && !flitfifo_empty(&r->corner_fifo)
//        && (r->stall_north==0)
//        && (r->stall_y==0)
        && (r->my_turn_y || !r->request_from_north))
    {
        if (r->stall_y!=0) stat_stall_y++;
        else if (r->stall_north!=0) stat_stall_north++;
        else north = FROM_CORNER; // inject corner to north
    }



    // west to east decisions

    unsigned x_dest = X_FROM_RANK(r->in_west_fc.dest);
    y_dest = Y_FROM_RANK(r->in_west_fc.dest);
    if (r->in_west_valid && ((x_dest!=r->x) || r->stall_west)) {
        if (x_dest==r->x) stat_stall_west++;
        east = FORWARD_FROM_WEST; // foward west to east
    } else {
        if (r->in_west_valid && x_dest==r->x) {
            // arrived at corner
            if (y_dest==r->y) {
                // already in target row => no vertical transport necessary
                // => eject if possible
                if (flitfifo_full(&r->recv_fifo) || local!=EMPTY) {
                    // deflect if corner buffer is full or vertical ring already ejects
                    east = DEFLECT_FROM_WEST; // deflect west to east
                    stat_deflect_west++;
                } else {
                    local = EJECT_FROM_WEST; // directly eject west to local
                }
            } else {
                // insert in corner buffer, if possible
                if (flitfifo_full(&r->corner_fifo)) {
                    east = DEFLECT_FROM_WEST; 
                    stat_deflect_west++;
                } else {
                    corner = TURN_FROM_WEST; // insert in corner buffer
                }
            }
        }
    }

    if (!flitfifo_empty(&r->send_fifo)) {
        if (X_FROM_RANK(r->send_fifo.buf[r->send_fifo.first].dest) == r->x) {
            // next flit from send buffer is already at corner
            if (!flitfifo_full(&r->corner_fifo) && corner==EMPTY) {
                // put directly into corner buffer, if there is space
                // and there was no turn from the west port
                corner = INJECT_FROM_LOCAL;
            }
        } else {
            if (east==EMPTY
//                && (r->stall_east==0)
//                && (r->stall_x==0)
                && (r->my_turn_x || !r->request_from_east)) 
            {
                if (r->stall_x!=0) stat_stall_x++;
                else if (r->stall_east!=0) stat_stall_east++;
                else east = INJECT_FROM_LOCAL; // inject from local to east
            }
        }
    }

    // set output signals and manage buffers

    switch (north) {
        case FORWARD_FROM_SOUTH:
            r->out_north_valid = true;
            r->out_north_deflected = r->in_south_deflected;
            r->out_north_fc = r->in_south_fc;
            break;
        case DEFLECT_FROM_SOUTH:
            r->stall_south = conf_noc_height; // don't eject for one round
            r->out_north_valid = true;
            r->out_north_deflected = true;
            r->out_north_fc = r->in_south_fc;
            break;
        case FROM_CORNER:
            flitfifo_remove(&r->corner_fifo, &r->out_north_fc);
            r->out_north_valid = true;
            r->out_north_deflected = false;
            r->my_turn_y = false;
            break;
        case EMPTY:
        default:
            r->out_north_valid = false;
            r->out_north_deflected = false;
            r->my_turn_y = true; // next time it is my turn
            r->request_from_north = false;
            break;
    }
    switch (east) {
        case FORWARD_FROM_WEST:
            r->out_east_valid = true;
            r->out_east_deflected = r->in_west_deflected;
            r->out_east_fc = r->in_west_fc;
            break;
        case DEFLECT_FROM_WEST:
            r->stall_west = conf_noc_width; // don't eject for one round
            r->out_east_valid = true;
            r->out_east_deflected = true;
            r->out_east_fc = r->in_west_fc;
            break;
        case INJECT_FROM_LOCAL:
            flitfifo_remove(&r->send_fifo, &r->out_east_fc);
            r->out_east_valid  = true;
            r->out_east_deflected = false;
            r->my_turn_x = false;
            break;
        case EMPTY:
        default:
            r->out_east_valid = false;
            r->out_east_deflected = false;
            r->my_turn_x = true; // next time it's me
            r->request_from_east = false;
            break;
    }
    switch (local) {
        case EJECT_FROM_SOUTH:
            flitfifo_insert(&r->recv_fifo, &r->in_south_fc);
            break;
        case EJECT_FROM_WEST:
            flitfifo_insert(&r->recv_fifo, &r->in_west_fc);
            break;
    }
    switch (corner) {
        case TURN_FROM_WEST:
            flitfifo_insert(&r->corner_fifo, &r->in_west_fc);
            break;
        case INJECT_FROM_LOCAL:
        {
            flit_container2_t fc;
            flitfifo_remove(&r->send_fifo, &fc);
            flitfifo_insert(&r->corner_fifo, &fc);
        }
    }


/*

    // DOES NOT WORK CORRECTLY:

    // handle south to north request
    if (r->state_already_requested_south==false
        && (!flitfifo_empty(&r->corner_fifo) || r->request_from_north))
    {
        r->out_south_request = true;
        r->state_already_requested_south = true;
    } else {
        r->out_south_request = false;
    }


    if (!r->state_already_requested_west 
        && (!flitfifo_empty(&r->send_fifo) || r->request_from_east))
    {
        r->out_west_request = true;
        r->state_already_requested_west = true;
    } else {
        r->out_west_request = false;
    }
*/
}






// initialize the best effort paternoster interconnection simulator
void pnjm0_init(node_t *n)
{
    // set function pointers
    n->noc_send_flit         = pnjm0_send_flit;
    n->noc_recv_flit         = pnjm0_recv_flit;
    n->noc_sender_ready      = pnjm0_sender_ready;
    n->noc_probe_rank        = pnjm0_probe_rank;
    n->noc_probe_any         = pnjm0_probe_any;
//    n->noc_route_one_cycle   = pnjm0_route_one_cycle;

    pnjm0_context_t *r = fatal_malloc(sizeof(pnjm0_context_t));
    n->noc_context = r;

    r->stall_north = 0;
    r->stall_south = 0;
    r->stall_east = 0;
    r->stall_west = 0;
    r->stall_x = 0;
    r->stall_y = 0;

    r->in_north_request = false;
    r->in_south_valid = false;
    r->in_south_deflected = false;
    r->in_east_request = false;
    r->in_west_valid = false;
    r->in_west_deflected = false;

    r->request_from_north = false;
    r->my_turn_y = false;
    r->state_already_requested_south = false;
    r->request_from_east = false;
    r->my_turn_x = false;
    r->state_already_requested_west = false;

    flitfifo_init(&r->send_fifo, conf_send_fifo_size);
    flitfifo_init(&r->recv_fifo, conf_recv_fifo_size);
    flitfifo_init(&r->corner_fifo, conf_corner_fifo_size);


    stat_stall_north = 0;
    stat_stall_east = 0;
    stat_stall_y = 0;
    stat_stall_x = 0;
    stat_stall_south = 0;
    stat_stall_west = 0;
    stat_deflect_south = 0;
    stat_deflect_west = 0;
}


void pnjm0_destroy(node_t *node)
{
    pnjm0_context_t *r = node->noc_context;
    flitfifo_destroy(&r->send_fifo);
    flitfifo_destroy(&r->recv_fifo);
    flitfifo_destroy(&r->corner_fifo);
    free(node->noc_context);
}


// Connect to the neigbour routers.
// All routers must be initalised before.
void pnjm0_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east)
{
    pnjm0_context_t *r = me->noc_context;
    r->x = x;
    r->y = y;
    r->rank = y*conf_noc_width+x;
    r->north = north->noc_context;
    r->south = south->noc_context;
    r->west = west->noc_context;
    r->east = east->noc_context;
}


void pnjm0_route_all(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    for (r=0; r<max_rank; r++) {
        router_one_cycle(nodes[r]->noc_context);
    }

    for (r=0; r<max_rank; r++) {
        pnjm0_context_t *self    = nodes[r]->noc_context;
        self->in_east_request    = self->east->out_west_request;
        self->in_west_valid      = self->west->out_east_valid;
        self->in_west_fc         = self->west->out_east_fc;
        self->in_west_deflected  = self->west->out_east_deflected;
        self->in_north_request   = self->north->out_south_request;
        self->in_south_valid     = self->south->out_north_valid;
        self->in_south_fc        = self->south->out_north_fc;
        self->in_south_deflected = self->south->out_north_deflected;
    }
}


void pnjm0_print_context(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    printf(" W   S |w e x s n y|SB CB RB (%lu)\n", nodes[0]->cycle);
    for (r=0; r<max_rank; r++) {
        pnjm0_context_t *self    = nodes[r]->noc_context;
        user_printf("%2lx%c%3lx%c|%1x%2x%2x%2x%2x%2x|%2lx%3lx%3lx\n",
            self->in_west_valid ? self->in_west_fc.dest : 255,
            self->in_west_deflected ? '*' : ' ',
            self->in_south_valid ? self->in_south_fc.dest : 255,
            self->in_south_deflected ? '*' : ' ',
            self->stall_west,
            self->stall_east,
            self->stall_x,
            self->stall_north,
            self->stall_south,
            self->stall_y,
            self->send_fifo.count,
            self->corner_fifo.count,
            self->recv_fifo.count
            );
    }
}


void pnjm0_print_stat()
{
    user_printf(
        "stall to empty x ring: %lu\n"
        "stall to empty y ring: %lu\n"
        "stall x inject:        %lu\n"
        "stall y inject:        %lu\n"
        "stall eject to corner: %lu\n"
        "stall eject to local:  %lu\n"
        "deflect x:             %lu\n"
        "deflect y:             %lu\n",
        stat_stall_x,
        stat_stall_y,
        stat_stall_east,
        stat_stall_north,
        stat_stall_west,
        stat_stall_south,
        stat_deflect_west,
        stat_deflect_south);
}
