/*
 * pnconfig.c
 * Highly configurable PaterNoster implementation
 *
 * RC/MC project
 *
 * TODO:
 * - flitslotdelay is used to delay incomming flits by 3 cycles. But the test,
 *   if the receive buffer is full, must also include the flits that are in this
 *   delay queue. Currently solved by recv_fifo_full_last_cycle, but this is a
 *   very ugly solution. Best solution: remove delay also in the VHDL model.
 *
 * - don't use conf_noc_width
 * - rename XXX_init() to XXX_init_router() and move memory allocation to macsim main
 */

#include "pnconfig.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


#define DEBUG(...)
//#define DEBUG printf



//#define conf_send_fifo_size     8
//#define conf_recv_fifo_size     (2*conf_max_rank)
//#define conf_corner_fifo_size   8


unsigned long stat_stall_east;
unsigned long stat_stall_north;
unsigned long stat_stall_x;
unsigned long stat_stall_y;
unsigned long stat_stall_west;
unsigned long stat_stall_south;
unsigned long stat_deflect_west;
unsigned long stat_deflect_south;

unsigned long stat_delay_north_not_empty; // bad name
    // unbuffered x-bypass could not directly inject in y-ring as there is no free slot
unsigned long stat_collision_cornerbuf_remove;
    // two removals from corner buffer at the same time
unsigned long stat_collision_eject;
    // two ejections at the same time and only one ejection port
unsigned long stat_recvbuf_full;
    // could not remove from corner buffer, because receive buffer is full

extern node_t *node;


bool pnconfig_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    flit_container2_t fc;
    fc.src = node->rank;
    fc.dest = dest;
    fc.flit = flit;
    pnconfig_context_t *ctx = node->noc_context;

    if (ctx->send_fifo_full_last_cycle) {
        return false;
    }

    if (flitfifo_insert(&ctx->send_fifo, &fc)) {
        ctx->send_fifo_full_last_cycle = 
            (ctx->send_fifo.count >= ctx->send_fifo.size - 1);
        return true;
    }

    return false;
}

bool pnconfig_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container2_t fc;
    pnconfig_context_t *ctx = node->noc_context;

    if (flitfifo_remove_by_rank(&ctx->recv_fifo, src, &fc)) {
        DEBUG("R %lu->%lu(%lx)\n", src, node->rank, fc.flit);
        *flit = fc.flit;
        ctx->recv_fifo_full_last_cycle = 
            ((ctx->recv_slot_delay.buf[1].src != RANK_EMPTY ? 1 : 0) +
            ctx->recv_fifo.count >= ctx->recv_fifo.size - 1);
        return true;
    }
    return false;
}

bool pnconfig_sender_ready(node_t *node)
{
    return !((pnconfig_context_t *)node->noc_context)->send_fifo_full_last_cycle;
}

bool pnconfig_probe_rank(node_t *node, rank_t src)
{
    return flitfifo_find_rank(&((pnconfig_context_t *)node->noc_context)->recv_fifo, src) >= 0;
}

rank_t pnconfig_probe_any(node_t *node)
{
    return flitfifo_first_rank(&((pnconfig_context_t *)node->noc_context)->recv_fifo);
}


#define conf_inject_period_y (conf_noc_height/2)
#define conf_inject_period_x (conf_noc_width/2)
#define conf_starve_threshold (conf_noc_height/2)

/*
static inline bool recv_fifo_full(pnconfig_context_t *r)
{
    return flitfifo_full(&r->recv_fifo);
}
*/

static void router_one_cycle(pnconfig_context_t *r)
{
#define EMPTY 0
#define INJECT_FROM_LOCAL 1
#define FORWARD_FROM_WEST 2
#define DEFLECT_FROM_WEST 3

#define FROM_CORNER 4
#define FORWARD_FROM_SOUTH 5
#define DEFLECT_FROM_SOUTH 6

#define EJECT_FROM_SOUTH 7
#define EJECT_FROM_WEST 8
#define EJECT_FROM_CORNER 9

#define TURN_FROM_WEST 10

    // stall policy


    if (r->stall_south!=0) r->stall_south--; 
        // don't eject for 1 round after an ejection was rejected
    if (r->in_south_valid && Y_FROM_RANK(r->in_south_fc.src) == r->y
           // don't stall if there is no y-bypass:
        && r->in_south_deflected)
    {
        r->stall_y = conf_noc_height;
    } else if (r->stall_y!=0) r->stall_y--;
        // don't inject as long as there are older flits in the ring that took
        // an extra round and were injected from here

    if (r->stall_west!=0) r->stall_west--;
        // don't eject for 1 round after an ejection was rejected
    if (r->in_west_valid && X_FROM_RANK(r->in_west_fc.src) == r->x
           // don't stall if there is no x-bypass:
        && r->in_west_deflected)
    {
        r->stall_x = conf_noc_width;
    } else if (r->stall_x!=0) r->stall_x--;
        // don't inject as long as there are older flits in the ring that took
        // an extra round and were injected from here


    // don't inject for 1 round after any deflected flit came by
    if (conf_stall_y==CONF_STALL_EXP) {
        if (r->in_south_deflected) {
            rank_t y = Y_FROM_RANK(r->in_south_fc.dest);
            if (r->stall_per_y_dest[y]==0) r->stall_per_y_dest[y] = conf_noc_height+1;
        }
        rank_t i;
        for (i=0; i<conf_noc_height; i++) {
            if (r->stall_per_y_dest[i]!=0) r->stall_per_y_dest[i]--;
        }
    } else { // conf_stall_y==CONF_STALL_CHEAP
        if (r->stall_north!=0) r->stall_north--; 
        else if (r->in_south_deflected) r->stall_north = conf_noc_height;
    }
#define STALL_NORTH(_dest) \
    (((conf_stall_y==CONF_STALL_CHEAP) && (r->stall_north!=0)) \
    || (r->stall_per_y_dest[Y_FROM_RANK(_dest)]!=0))

    // don't inject for 1 round after any deflected flit came by
    if (conf_stall_x==CONF_STALL_EXP) {
        if (r->in_west_deflected) {
            rank_t x = X_FROM_RANK(r->in_west_fc.dest);
            if (r->stall_per_x_dest[x]==0) r->stall_per_x_dest[x] = conf_noc_width+1;
        }
        rank_t i;
        for (i=0; i<conf_noc_width; i++) {
            if (r->stall_per_x_dest[i]!=0) r->stall_per_x_dest[i]--;
        }
    } else { // cinf_stall_y==CONF_STALL_CHEAP
        if (r->stall_east!=0) r->stall_east--;
        else if (r->in_west_deflected) r->stall_east = conf_noc_width;
    }
#define STALL_EAST(_dest) \
    (((conf_stall_x==CONF_STALL_CHEAP) && (r->stall_east!=0)) \
    || (r->stall_per_x_dest[X_FROM_RANK(_dest)]!=0))





    // inject policy


    if (conf_inject_y==CONF_INJECT_REQUEST && r->in_north_request) {
        r->request_from_north = true;
    }
    if (conf_inject_x==CONF_INJECT_REQUEST && r->in_east_request) {
        r->request_from_east = true;
    }
    if (r->in_south_valid==false) r->already_requested_south = false;
    if (r->in_west_valid==false) r->already_requested_west = false;

    bool injy_allowed;
    bool injx_allowed;
    switch (conf_inject_y) {
        case CONF_INJECT_NONE:
            injy_allowed = true;
            break;
        case CONF_INJECT_REQUEST:
            injy_allowed = (r->my_turn_y || !r->request_from_north);
            break;
        case CONF_INJECT_ALTERNATE:
            injy_allowed = (r->inject_counter_y>=conf_inject_period_y);
            if (r->inject_counter_y==0) r->inject_counter_y = 2*conf_inject_period_y;
            r->inject_counter_y--;
            break;
        case CONF_INJECT_THROTTLE:
            injy_allowed = !r->in_throttle_y;
            break;
        default: fatal("Unknown y injection policy");
    }
    switch (conf_inject_x) {
        case CONF_INJECT_NONE:
            injx_allowed = true;
            break;
        case CONF_INJECT_REQUEST:
            injx_allowed = (r->my_turn_x || !r->request_from_east);
            break;
        case CONF_INJECT_ALTERNATE:
            injx_allowed = (r->inject_counter_x>=conf_inject_period_x);
            if (r->inject_counter_x==0) r->inject_counter_x = 2*conf_inject_period_x;
            r->inject_counter_x--;
            break;
        case CONF_INJECT_THROTTLE:
            injx_allowed = !r->in_throttle_x;
            break;
        default: fatal("Unknown y injection policy");
    }



    // south to north decisions
    unsigned north = EMPTY;
    unsigned local = EMPTY;

    unsigned y_dest = Y_FROM_RANK(r->in_south_fc.dest);
    if (r->in_south_valid) {
        if (y_dest!=r->y) {
            north = FORWARD_FROM_SOUTH; // forward south to north
        } else if (r->stall_south) {
            stat_stall_south++;
            north = DEFLECT_FROM_SOUTH;
        } else if (r->recv_fifo_full_last_cycle) {
            stat_deflect_south++;
            r->stall_south = conf_noc_height; // don't eject for one round
            north = DEFLECT_FROM_SOUTH; // forward south to north, mark as deflected
        } else {
            local = EJECT_FROM_SOUTH; // eject south to local
        }
    }


    if (!flitfifo_empty(&r->corner_fifo) && injy_allowed) {
        rank_t dest = r->corner_fifo.buf[r->corner_fifo.first].dest;
        if ((conf_bypass_y!=CONF_BYPASS_BUF && conf_bypass_y!=CONF_BYPASS_2BUF)
            || (dest != r->rank))
            // with buffered y-bypass flits are directly ejected from the corner buffer
            // this is done later in the code
        {
            if (r->stall_y!=0) stat_stall_y++;
            else if (STALL_NORTH(dest)) stat_stall_north++;
            else if ((conf_bypass_y==CONF_BYPASS_NONE) && r->stall_south!=0) ; 
                // special case without bypass:
                // don't inject, when flits where deflected
                // reason: ring may not be altered, until the first deflected flit
                // is correctly ejected
                // I don't understand exactly why, but it works
            else if (north!=EMPTY) r->starve_counter_y++;
            else north = FROM_CORNER; // inject corner to north
        }
    }



    // west to east decisions
    unsigned east = EMPTY;
    unsigned corner = EMPTY;
    unsigned local2 = EMPTY; // only if double ejection allowed

    unsigned x_dest = X_FROM_RANK(r->in_west_fc.dest);
    y_dest = Y_FROM_RANK(r->in_west_fc.dest);

    if (r->in_west_valid) {
        if (x_dest!=r->x) { // target corner not yet reached
            east = FORWARD_FROM_WEST;
        } else if (r->stall_west) { // within a ejection stalling round due to prior deflection
            stat_stall_west++;
            east = DEFLECT_FROM_WEST;
        } else if (y_dest==r->y) { // already in target row, no vertical transport necessary
            switch (conf_bypass_y) {
                case CONF_BYPASS_UNBUF:
                    if (r->recv_fifo_full_last_cycle || local!=EMPTY) {
                        // deflect x if receive buffer is full or y-ring already ejects
                        r->stall_west = conf_noc_width; // don't eject for one round
                        east = DEFLECT_FROM_WEST; // deflect west to east
                    } else {
                        local = EJECT_FROM_WEST; // directly eject west to local
                    }
                    break;
                case CONF_BYPASS_2UNBUF:
// FIXME: correctly take into account that flits might be in the 3-cycle 
// revc_slot_delay
                    if (((local==EMPTY) && flitfifo_full(&r->recv_fifo)) // not 1 free entry
                        || ((r->recv_fifo.count+2) > r->recv_fifo.size)) // no 2 free entries
                    {
                        // deflect x if receive buffer is full
                        r->stall_west = conf_noc_width; // don't eject for one round
                        east = DEFLECT_FROM_WEST; // deflect west to east
                    } else {
                        local2 = EJECT_FROM_WEST; // directly eject west to local
                    }
                    break;
                case CONF_BYPASS_NONE:
                    // here same as buffered, only difference that noc ejection from corner
                    // se below
                case CONF_BYPASS_BUF:
                case CONF_BYPASS_2BUF:
                    if (flitfifo_full(&r->corner_fifo)) {
                        stat_deflect_west++;
                        r->stall_west = conf_noc_width; // don't eject for one round
                        east = DEFLECT_FROM_WEST; 
                    } else {
                        corner = TURN_FROM_WEST; // insert in corner buffer
                    }
                    break;
                default: fatal("Unknown y bypass policy");
            }
        } else if (flitfifo_full(&r->corner_fifo)) {
            stat_deflect_west++;
            r->stall_west = conf_noc_width; // don't eject for one round
            east = DEFLECT_FROM_WEST; 
        } else {
            corner = TURN_FROM_WEST; // insert in corner buffer
        }
    }

    // check if flit in corner buffer can direcly be ejected
    // only neccessary if buffered y bypass
    if (!flitfifo_empty(&r->corner_fifo)
        && (r->corner_fifo.buf[r->corner_fifo.first].dest == r->rank))
    {
         if (conf_bypass_y==CONF_BYPASS_BUF) {
//            if (flitfifo_full(&r->recv_fifo)) stat_recvbuf_full++;
            if (r->recv_fifo_full_last_cycle) stat_recvbuf_full++;
            else if (north==FROM_CORNER) stat_collision_cornerbuf_remove++;
            else if (local!=EMPTY) stat_collision_eject++;
            else local = EJECT_FROM_CORNER;
        } else  if (conf_bypass_y==CONF_BYPASS_2BUF) {
// FIXME: correctly take into account that flits might be in the 3-cycle 
// revc_slot_delay
            if (((local==EMPTY) && flitfifo_full(&r->recv_fifo)) // not 1 free entry
                || ((r->recv_fifo.count+2) > r->recv_fifo.size)) // no 2 free entries
            {
                stat_recvbuf_full++;
            } else if (north==FROM_CORNER) stat_collision_cornerbuf_remove++;
            else local2 = EJECT_FROM_CORNER;
        }
    }

    if (!flitfifo_empty(&r->send_fifo)) {
        // x bypass
        rank_t dest = r->send_fifo.buf[r->send_fifo.first].dest;
        if (X_FROM_RANK(dest) == r->x) {
            // next flit from send buffer is already at corner
            switch (conf_bypass_x) {
                case CONF_BYPASS_NONE:
                    if (injx_allowed) {
                        if (r->stall_x!=0) stat_stall_x++;
                        else if (STALL_EAST(dest)) stat_stall_east++;
                        else if (r->stall_west!=0) ; 
                            // special case without bypass:
                            // don't inject, when flits where deflected
                            // reason: ring may not be altered, until the first deflected flit
                            // is correctly ejected
                            // I don't understand exactly why, but it works
                        else if (east!=EMPTY) r->starve_counter_x++;
                        else east = INJECT_FROM_LOCAL; // inject from local to east
                    }
                    break;
                case CONF_BYPASS_UNBUF:
                    if (injy_allowed) {
                        // my turn
                        if (r->stall_y!=0) stat_stall_y++;
                        else if (STALL_NORTH(dest)) stat_stall_north++;
                        else if (north!=EMPTY) r->starve_counter_y++;
                        else north=INJECT_FROM_LOCAL;
                    }
                    break;
                case CONF_BYPASS_BUF:
                    if (!flitfifo_full(&r->corner_fifo) && corner==EMPTY) {
                        // put directly into corner buffer, if there is space
                        // and there was no turn from the west port
                        corner = INJECT_FROM_LOCAL;
                    }
                    break;
                default: fatal("Unknown x bypass policy");
            }
        } else {
            if (injx_allowed) {
                if (r->stall_x!=0) stat_stall_x++;
                else if (STALL_EAST(dest)) stat_stall_east++;
                else if (east!=EMPTY) r->starve_counter_x++;
                else east = INJECT_FROM_LOCAL; // inject from local to east
            }
        }
    }

    r->send_fifo_full_last_cycle = (r->send_fifo.count >= r->send_fifo.size - 1);

    // set output signals and manage buffers

    switch (north) {
        case FORWARD_FROM_SOUTH:
            r->out_north_valid = true;
            r->out_north_deflected = r->in_south_deflected;
            r->out_north_fc = r->in_south_fc;
            break;
        case DEFLECT_FROM_SOUTH:
            r->out_north_valid = true;
            r->out_north_deflected = true;
            r->out_north_fc = r->in_south_fc;
            break;
        case INJECT_FROM_LOCAL:
            flitfifo_remove(&r->send_fifo, &r->out_north_fc);
            r->out_north_valid  = true;
            r->out_north_deflected = false;
            r->my_turn_y = false;
            r->starve_counter_y = 0;
            r->out_starved_y = false;
            break;
        case FROM_CORNER:
            flitfifo_remove(&r->corner_fifo, &r->out_north_fc);
            r->out_north_valid = true;
            r->out_north_deflected = false;
            r->my_turn_y = false;
            r->starve_counter_y = 0;
            break;
        case EMPTY:
        default:
            r->out_north_valid = false;
            r->out_north_deflected = false;
            r->my_turn_y = true; // next time it is my turn
            r->starve_counter_y = 0;
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
            r->out_east_valid = true;
            r->out_east_deflected = true;
            r->out_east_fc = r->in_west_fc;
            break;
        case INJECT_FROM_LOCAL:
            flitfifo_remove(&r->send_fifo, &r->out_east_fc);
            r->out_east_valid  = true;
            r->out_east_deflected = false;
            r->my_turn_x = false;
            r->starve_counter_x = 0;
            break;
        case EMPTY:
        default:
            r->out_east_valid = false;
            r->out_east_deflected = false;
            r->my_turn_x = true; // next time it's me
            r->request_from_east = false;
            r->starve_counter_x = 0;
            break;
    }
    switch (local) {
        case EJECT_FROM_SOUTH:
            flitslotdelay_insert(&r->recv_slot_delay, &r->in_south_fc);
            break;
        case EJECT_FROM_WEST:
            flitslotdelay_insert(&r->recv_slot_delay, &r->in_west_fc);
            break;
        case EJECT_FROM_CORNER:
        {
            flit_container2_t fc;
            flitfifo_remove(&r->corner_fifo, &fc);
            flitslotdelay_insert(&r->recv_slot_delay, &fc);
            break;
        }
    }
    switch (local2) {
        case EJECT_FROM_WEST:
            assert(flitfifo_insert(&r->recv_fifo, &r->in_west_fc));
            break;
        case EJECT_FROM_CORNER:
        {
            flit_container2_t fc;
            flitfifo_remove(&r->corner_fifo, &fc);
            assert(flitfifo_insert(&r->recv_fifo, &fc));
            break;
        }
    }
    switch (corner) {
        case TURN_FROM_WEST:
            assert(flitfifo_insert(&r->corner_fifo, &r->in_west_fc));
            break;
        case INJECT_FROM_LOCAL:
        {
            flit_container2_t fc;
            flitfifo_remove(&r->send_fifo, &fc);
            assert(flitfifo_insert(&r->corner_fifo, &fc));
            break;
        }
    }


    if (!r->already_requested_south
        && (!flitfifo_empty(&r->corner_fifo) || r->request_from_north))
    {
        r->out_south_request = true;
        r->already_requested_south = true;
    } else {
        r->out_south_request = false;
    }
    if (!r->already_requested_west 
        && (!flitfifo_empty(&r->send_fifo) || r->request_from_east))
    {
        r->out_west_request = true;
        r->already_requested_west = true;
    } else {
        r->out_west_request = false;
    }

    r->out_starved_y = (r->starve_counter_y >= conf_starve_threshold);
    r->out_starved_x = (r->starve_counter_x >= conf_starve_threshold);

    flit_container2_t fc;
    if (flitslotdelay_step(&r->recv_slot_delay, &fc)) {
        assert(flitfifo_insert(&r->recv_fifo, &fc));
    }

    r->recv_fifo_full_last_cycle = 
        ((r->recv_slot_delay.buf[0].src != RANK_EMPTY ? 1 : 0) + 
        r->recv_fifo.count >= r->recv_fifo.size - 1);

	// decisions are only saved for VHDL dumping
    r->decision_north = north;
    r->decision_east = east;
    r->decision_local = local;
    r->decision_local2 = local2;
    r->decision_corner = corner;
}






// initialize the best effort paternoster interconnection simulator
void pnconfig_init(node_t *n)
{
    rank_t i;

    // set function pointers
    n->noc_send_flit         = pnconfig_send_flit;
    n->noc_recv_flit         = pnconfig_recv_flit;
    n->noc_sender_ready      = pnconfig_sender_ready;
    n->noc_probe_rank        = pnconfig_probe_rank;
    n->noc_probe_any         = pnconfig_probe_any;
//    n->noc_route_one_cycle   = pnconfig_route_one_cycle;

    pnconfig_context_t *r = fatal_malloc(sizeof(pnconfig_context_t));
    n->noc_context = r;

    r->decision_north = EMPTY;
    r->decision_east = EMPTY;
    r->decision_local = EMPTY;
    r->decision_local2 = EMPTY;
    r->decision_corner = EMPTY;

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
    r->already_requested_south = false;
    r->request_from_east = false;
    r->my_turn_x = false;
    r->already_requested_west = false;
    r->starve_counter_y = 0;
    r->in_throttle_y = false;
    r->out_starved_y = false;
    r->starve_counter_x = 0;
    r->in_throttle_x = false;
    r->out_starved_x = false;

    flitfifo_init(&r->send_fifo, conf_send_fifo_size);
    flitfifo_init(&r->recv_fifo, conf_recv_fifo_size);
    flitfifo_init(&r->corner_fifo, conf_corner_fifo_size);
    flitslotdelay_init(&r->recv_slot_delay, 3);

    r->recv_fifo_full_last_cycle = false;
    r->send_fifo_full_last_cycle = false;

    r->stall_per_y_dest = fatal_malloc(conf_noc_height*sizeof(unsigned));
    r->stall_per_x_dest = fatal_malloc(conf_noc_width*sizeof(unsigned));
    for (i=0; i<conf_noc_height; i++) r->stall_per_y_dest[i] = 0;
    for (i=0; i<conf_noc_width; i++) r->stall_per_x_dest[i] = 0;

    stat_stall_north = 0;
    stat_stall_east = 0;
    stat_stall_y = 0;
    stat_stall_x = 0;
    stat_stall_south = 0;
    stat_stall_west = 0;
    stat_deflect_south = 0;
    stat_deflect_west = 0;

    stat_delay_north_not_empty = 0;
    stat_collision_cornerbuf_remove = 0;
    stat_collision_eject = 0;
    stat_recvbuf_full = 0;
}


void pnconfig_destroy(node_t *node)
{
    pnconfig_context_t *r = node->noc_context;
    flitfifo_destroy(&r->send_fifo);
    flitfifo_destroy(&r->recv_fifo);
    flitfifo_destroy(&r->corner_fifo);
    free(r->stall_per_y_dest);
    free(r->stall_per_x_dest);
    free(node->noc_context);
}


// Connect to the neigbour routers.
// All routers must be initalised before.
void pnconfig_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east)
{
    pnconfig_context_t *r = me->noc_context;
    r->x = x;
    r->y = y;
    r->rank = y*conf_noc_width+x;
    r->north = north->noc_context;
    r->south = south->noc_context;
    r->west = west->noc_context;
    r->east = east->noc_context;

    // alternate between nodes
    r->inject_counter_x = (x&1)*conf_inject_period_x;
    r->inject_counter_y = (y&1)*conf_inject_period_y;
}


void pnconfig_route_all(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    for (r=0; r<max_rank; r++) {
        router_one_cycle(nodes[r]->noc_context);
    }

    for (r=0; r<max_rank; r++) {
        pnconfig_context_t *self    = nodes[r]->noc_context;
        self->in_east_request    = self->east->out_west_request;
        self->in_west_valid      = self->west->out_east_valid;
        self->in_west_fc         = self->west->out_east_fc;
        self->in_west_deflected  = self->west->out_east_deflected;
        self->in_north_request   = self->north->out_south_request;
        self->in_south_valid     = self->south->out_north_valid;
        self->in_south_fc        = self->south->out_north_fc;
        self->in_south_deflected = self->south->out_north_deflected;
    }

#define NODE_XY(_x,_y) ((pnconfig_context_t *)(&nodes[_y*conf_noc_width+_x]->noc_context))

    if (conf_inject_y==CONF_INJECT_THROTTLE) {
        rank_t x;
        for (x=0; x<conf_noc_width; x++) {
            bool any_starved = false;
            rank_t y;
            for (y=0; y<conf_noc_height; y++) {
                if (NODE_XY(x, y)->out_starved_y) {
                    any_starved = true;
                    break;
                }
            }
            if (NODE_XY(x, y)->in_throttle_y) {
                if (any_starved==false) {
                    // de-throttle if throttled and no one starved
                    for (x=0; x<conf_noc_width; x++) {
                        NODE_XY(x, y)->in_throttle_y = false;
                    }
                }
            } else {
                if (any_starved==true) {
                    // throttle if not throttled yet and a node is starved
                    for (x=0; x<conf_noc_width; x++) {
                        NODE_XY(x, y)->in_throttle_y = true;
                    }
                }
            }
        }
    }

    if (conf_inject_x==CONF_INJECT_THROTTLE) {
        rank_t y;
        for (y=0; y<conf_noc_height; y++) {
            bool any_starved = false;
            rank_t x;
            for (x=0; x<conf_noc_width; x++) {
                if (NODE_XY(x, y)->out_starved_x) {
                    any_starved = true;
                    break;
                }
            }
            if (NODE_XY(x, y)->in_throttle_x) {
                if (any_starved==false) {
                    // de-throttle if throttled and no one starved
                    for (x=0; x<conf_noc_width; x++) {
                        NODE_XY(x, y)->in_throttle_x = false;
                    }
                }
            } else {
                if (any_starved==true) {
                    // throttle if not throttled yet and a node is starved
                    for (x=0; x<conf_noc_width; x++) {
                        NODE_XY(x, y)->in_throttle_x = true;
                    }
                }
            }
        }
    }

}


static void strflit(char *s, bool valid, bool defl, flit_container2_t fc)
{
    if (!valid) strcpy(s, "         ");
    else sprintf(s, "%2lx>%2lx[%016lx]%c", fc.src, fc.dest, fc.flit, defl ? '*' : ' ');
}

void pnconfig_print_context(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    printf("    W        S      |w e x s n y|SB CB RB (%lu)\n", nodes[0]->cycle);
    for (r=0; r<max_rank; r++) {
        pnconfig_context_t *self    = nodes[r]->noc_context;
        char w[32], s[32];
        strflit(w, self->in_west_valid, self->in_west_deflected, self->in_west_fc);
        strflit(s, self->in_south_valid, self->in_south_deflected, self->in_south_fc);
        user_printf("%02x%s%s|%1x%2x%2x%2x%2x%2x|%2lx%3lx%3lx", r,
            &w, &s,
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
        printf("\n");
    }

//    pnconfig_context_t *n17 = nodes[0x17]->noc_context;
//    flitfifo_verbose(&n17->corner_fifo);
//    printf("\n");
//
//    flitfifo_verbose(&n17->recv_fifo);
//    printf("\n");
}


void pnconfig_print_stat()
{
    user_printf(
        "don't inject until x ring is empty:           %lu\n"
        "don't inject until y ring is empty:           %lu\n"
        "don't inject due to deflected flit in x ring: %lu\n"
        "don't inject due to deflected flit in y ring: %lu\n"
        "don't eject to corner:                        %lu\n"
        "don't eject to local:                         %lu\n"
        "deflect x:                                    %lu\n"
        "deflect y:                                    %lu\n"
        "eject collision:                              %lu\n",
        stat_stall_x,
        stat_stall_y,
        stat_stall_east,
        stat_stall_north,
        stat_stall_west,
        stat_stall_south,
        stat_deflect_west,
        stat_deflect_south,
        stat_collision_eject);
}

static char * decision_name(unsigned decision)
{
    switch (decision)
    {
        case EMPTY: return "EMPTY";
        case INJECT_FROM_LOCAL: return "INJECT_FROM_LOCAL";
        case FORWARD_FROM_WEST: return "FORWARD_FROM_WEST";
        case DEFLECT_FROM_WEST: return "DEFLECT_FROM_WEST";
        case FROM_CORNER: return "FROM_CORNER";
        case FORWARD_FROM_SOUTH: return "FORWARD_FROM_SOUTH";
        case DEFLECT_FROM_SOUTH: return "DEFLECT_FROM_SOUTH";
        case EJECT_FROM_SOUTH: return "EJECT_FROM_SOUTH";
        case EJECT_FROM_WEST: return "EJECT_FROM_WEST";
        case EJECT_FROM_CORNER: return "EJECT_FROM_CORNER";
        case TURN_FROM_WEST: return "TURN_FROM_WEST";
    }
    return "UNKNOWN";
}

void pnconfig_dump_context(const char *file, node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    FILE *out = fopen(file, "w");
    if (!out) {
        fatal("Could not open '%s'", file);
    }
    for (r=0; r<max_rank; r++) {
        pnconfig_context_t *self    = nodes[r]->noc_context;
        flit_container2_t *west = &self->in_west_fc;
        flit_container2_t *south = &self->in_south_fc;
        // Ports
        fprintf(out, "%ld w %016lx s %016lx\n", r, self->in_west_valid ? west->flit : 0, self->in_south_valid ? south->flit : 0);
        // SB
        int pos = self->send_fifo.first;
        for (int i = 0; i < self->send_fifo.count; i++) {
            unsigned x = X_FROM_RANK(self->send_fifo.buf[pos].dest);
            unsigned y = Y_FROM_RANK(self->send_fifo.buf[pos].dest);
            if (i == 0) {
                fprintf(out, "%ld sb %x%016lx", r, (x << 2 | y), self->send_fifo.buf[pos].flit);
            } else {
                fprintf(out, " %x%016lx", (x << 2 | y), self->send_fifo.buf[pos].flit);
            }
            pos++;
            if (pos == self->send_fifo.size) {
                pos = 0;
            }
        }

        if (self->send_fifo.count != 0) {
            fprintf(out, "\n");
        }


        /*// RB (TODO: fix)
        pos = self->recv_fifo.first;
        for (int i = 0; i < self->recv_fifo.count; i++) {
            unsigned x = X_FROM_RANK(self->recv_fifo.buf[pos].src);
            unsigned y = Y_FROM_RANK(self->recv_fifo.buf[pos].src);
            if (i == 0) {
                fprintf(out, "%ld rb %x%016lx", r, (x << 2 | y), self->recv_fifo.buf[pos].flit);
            } else {
                fprintf(out, " %x%016lx", (x << 2 | y), self->recv_fifo.buf[pos].flit);
            }
            pos++;
            if (pos == self->recv_fifo.size) {
                pos = 0;
            }
        }

        if (self->recv_fifo.count != 0) {
            fprintf(out, "\n");
        }*/

        // Routing decisions.
        fprintf(out, "%ld n %s e %s l %s c %s\n", r, decision_name(self->decision_north), decision_name(self->decision_east), decision_name(self->decision_local), decision_name(self->decision_corner));
        // Stall counters.
        fprintf(out, "%ld sw %x se %x sx %x sn %x ss %x sy %x\n", r, self->stall_west, self->stall_east, self->stall_x, self->stall_north, self->stall_south, self->stall_y);
        fprintf(out, "%ld rbf %x\n", r, self->recv_fifo_full_last_cycle ? 1 : 0);
    }
    fclose(out);
}

void pnconfig_log_traffic(const char *file, node_t *nodes[], rank_t max_rank)
{
    FILE *out;
    rank_t r;
    out = fopen(file,"a");
    if( out == NULL){
        fatal("Could not open the specified log file");
    }
    for(r=0;r<max_rank;r++){
        pnconfig_context_t *self = nodes[r]->noc_context;
        fprintf(out,"%-20s\t%-20s\t%-20s\t%-20s\n",decision_name(self->decision_north), decision_name(self->decision_east), decision_name(self->decision_local), decision_name(self->decision_corner));
    }
    fclose(out);
}