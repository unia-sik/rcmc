#ifndef _PNCONFIG_H
#define _PNCONFIG_H

#include "node.h"
#include "share.h"
#include "flitfifo.h"
#include "flitslotdelay.h"

typedef struct pnconfig_context_s
{
    rank_t              rank;
    unsigned            x;
    unsigned            y;
    struct pnconfig_context_s *west, *east, *south, *north;

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

    unsigned            decision_north;
    unsigned            decision_east;
    unsigned            decision_local;
    unsigned            decision_local2;
    unsigned            decision_corner;

    unsigned            stall_north; // don't inject to north
    unsigned            stall_south; // don't eject from south
    unsigned            stall_y; // don't inject until ring is empty
    unsigned            stall_east; // don't inject to east
    unsigned            stall_west; // don't eject from west
    unsigned            stall_x; // don't inject until ring is empty
    unsigned            *stall_per_y_dest;
    unsigned            *stall_per_x_dest;

    bool                request_from_north;
    bool                my_turn_y;
    bool                already_requested_south;
    unsigned            inject_counter_y;
    unsigned            starve_counter_y;
    bool                in_throttle_y;
    bool                out_starved_y;

    bool                request_from_east;
    bool                my_turn_x;
    bool                already_requested_west;
    unsigned            inject_counter_x;
    unsigned            starve_counter_x;
    bool                in_throttle_x;
    bool                out_starved_x;

    flitfifo_t          send_fifo;
    flitfifo_t          recv_fifo;
    flitfifo_t          corner_fifo;

    flitslotdelay_t     recv_slot_delay;
    bool                recv_fifo_full_last_cycle;
    bool                send_fifo_full_last_cycle;
} pnconfig_context_t;


bool pnconfig_send_flit(node_t *node, rank_t dest, flit_t flit);
bool pnconfig_recv_flit(node_t *node, rank_t src, flit_t *flit);
bool pnconfig_sender_ready(node_t *node);
bool pnconfig_probe_rank(node_t *node, rank_t src);
rank_t pnconfig_probe_any(node_t *node);

//void pnconfig_route_one_cycle(node_t *node);
void pnconfig_init(node_t *node);
void pnconfig_destroy(node_t *node);
void pnconfig_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east);
void pnconfig_route_all(node_t *nodes[], rank_t max_rank);
void pnconfig_print_context(node_t *nodes[], rank_t max_rank);
void pnconfig_print_stat();

void pnconfig_dump_context(const char *file, node_t *nodes[], rank_t max_rank);
void pnconfig_log_traffic(const char *file, node_t *nodes[], rank_t max_rank);

#endif
