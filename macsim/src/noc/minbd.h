#ifndef _MINBD_H
#define _MINBD_H

#include "node.h"
#include "share.h"
#include "flitfifo.h"
#include "flitqueue.h"

#define MAX_CORNER_FIFO_SIZE 8

typedef struct minbd_context_s
{
    rank_t              rank;
    unsigned            x;
    unsigned            y;
    struct minbd_context_s *west, *east, *south, *north;

    bool                in_valid[4];
    flit_container2_t   in_fc[4];

    bool                out_valid[4];
    flit_container2_t   out_fc[4];

    bool                s2_valid[4];
    flit_container2_t   s2_fc[4];

    unsigned            redir_counter;

    unsigned long       *send_no;
    unsigned long       *recv_no;
    flitfifo_t          send_fifo;
    fc_queue_t          recv_fifo;
    flitfifo_t          side_fifo;
} minbd_context_t;


bool minbd_send_flit(node_t *node, rank_t dest, flit_t flit);
bool minbd_recv_flit(node_t *node, rank_t src, flit_t *flit);
bool minbd_sender_ready(node_t *node);
bool minbd_probe_rank(node_t *node, rank_t src);
rank_t minbd_probe_any(node_t *node);

//void minbd_route_one_cycle(node_t *node);
void minbd_init(node_t *node);
void minbd_destroy(node_t *node);
void minbd_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east);
void minbd_route_all(node_t *nodes[], rank_t max_rank);
void minbd_print_context(node_t *nodes[], rank_t max_rank);
void minbd_print_stat();

#endif
