#include "pnoo_msg.h"

#include <string.h>

void pnoo_msg_clear(pnoo_msg_t* msg)
{
    memset(msg, 0, sizeof(pnoo_msg_t));
    msg->valid = false;
}

pnoo_msg_t pnoo_msg_init(const rank_t src, const rank_t dest)
{
    pnoo_msg_t result;
    
    result.valid = true;    
    result.flit.dest = dest;
    result.flit.src = src;
    
    return result; 
}


pnoo_msg_t pnoo_msg_init_with_data(const rank_t src, const rank_t dest, const flit_t data)
{
    pnoo_msg_t result = pnoo_msg_init(src, dest);
    memcpy(&result.flit.payload, &data, sizeof(flit_t));
    
    return result; 
}


