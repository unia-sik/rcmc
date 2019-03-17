#pragma once

#include "share.h"

typedef struct {    
    flit_container_t flit;
    bool valid;
} pnoo_msg_t;

void pnoo_msg_clear(pnoo_msg_t* msg);
pnoo_msg_t pnoo_msg_init(const rank_t src, const rank_t dest);
pnoo_msg_t pnoo_msg_init_with_data(const rank_t src, const rank_t dest, const flit_t data);
