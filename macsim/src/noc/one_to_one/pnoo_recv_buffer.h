#pragma once

#include "pnoo_msg.h"

typedef struct {
    pnoo_msg_t* entries;
    int head;
    int tail;
    int end;
} pnoo_recv_buffer_t;

void pnoo_recv_buffer_init(pnoo_recv_buffer_t* buffer, const int size);
void pnoo_recv_buffer_destroy(pnoo_recv_buffer_t* buffer);

void pnoo_recv_buffer_push(pnoo_recv_buffer_t* buffer, const pnoo_msg_t* msg);
void pnoo_recv_buffer_erase(pnoo_recv_buffer_t* buffer, const int index);

int pnoo_recv_buffer_search_from_rank(const pnoo_recv_buffer_t* buffer, const rank_t src);
int pnoo_recv_buffer_search_from_any(const pnoo_recv_buffer_t* buffer);

flit_t pnoo_recv_buffer_get_flit(const pnoo_recv_buffer_t* buffer, const int index);
rank_t pnoo_recv_buffer_get_src(const pnoo_recv_buffer_t* buffer, const int index);
