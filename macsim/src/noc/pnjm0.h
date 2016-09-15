/*
 * pnbe2.h
 *
 *  Created on: 02.04.2015
 *      Author: gorlorom
 */

#ifndef _PNJM0_H
#define _PNJM0_H

#include "node.h"
#include "share.h"
#include "flitfifo.h"

#define MAX_CORNER_FIFO_SIZE 8

typedef struct pnjm0_context_s
{
    rank_t              rank;
    unsigned            x;
    unsigned            y;
    struct pnjm0_context_s *west, *east, *south, *north;

    bool                in_north_request;
    bool                in_south_valid;
    bool                in_south_deflected;
    flit_container2_t   in_south_fc;

    bool                in_east_request;
    bool                in_west_valid;
    bool                in_west_deflected;
    flit_container2_t   in_west_fc;

    bool                out_south_request;
    bool                out_north_valid;
    bool                out_north_deflected;
    flit_container2_t   out_north_fc;

    bool                out_west_request;
    bool                out_east_valid;
    bool                out_east_deflected;
    flit_container2_t   out_east_fc;

    bool                request_from_east;
    bool                my_turn_x;
    bool                state_already_requested_west;

    bool                request_from_north;
    bool                my_turn_y;
    bool                state_already_requested_south;

    unsigned            stall_north; // don't inject to north
    unsigned            stall_south; // don't eject from south
    unsigned            stall_y; // don't inject until ring is empty
    unsigned            stall_east; // don't inject to east
    unsigned            stall_west; // don't eject from west
    unsigned            stall_x; // don't inject until ring is empty

    flitfifo_t          send_fifo;
    flitfifo_t          recv_fifo;
    flitfifo_t          corner_fifo;

} pnjm0_context_t;


bool pnjm0_send_flit(node_t *node, rank_t dest, flit_t flit);
bool pnjm0_recv_flit(node_t *node, rank_t src, flit_t *flit);
bool pnjm0_sender_ready(node_t *node);
bool pnjm0_probe_rank(node_t *node, rank_t src);
rank_t pnjm0_probe_any(node_t *node);

//void pnjm0_route_one_cycle(node_t *node);
void pnjm0_init(node_t *node);
void pnjm0_destroy(node_t *node);
void pnjm0_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east);
void pnjm0_route_all(node_t *nodes[], rank_t max_rank);
void pnjm0_print_context(node_t *nodes[], rank_t max_rank);
void pnjm0_print_stat();

#endif
