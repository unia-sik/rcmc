#include "pnoo_recv_buffer.h"
#include <stdlib.h>
#include <string.h>

void pnoo_recv_buffer_init(pnoo_recv_buffer_t* buffer, const int size)
{
    buffer->entries = calloc(size, sizeof(pnoo_msg_t));
    buffer->head = 0;
    buffer->tail = 0;
    buffer->end = size;
}

void pnoo_recv_buffer_destroy(pnoo_recv_buffer_t* buffer)
{
    free(buffer->entries);
}

void pnoo_recv_buffer_push(pnoo_recv_buffer_t* buffer, const pnoo_msg_t* msg)
{
    if (buffer->tail < buffer->end) {
        buffer->entries[buffer->tail] = *msg;
        buffer->tail++;
    } else {
        fatal("Receive buffer full!\n");
    }
}

void pnoo_recv_buffer_erase(pnoo_recv_buffer_t* buffer, const int index)
{
    for (int i = index; i < buffer->end - 1; i++) {
        buffer->entries[i] = buffer->entries[i + 1];
    }
    
    buffer->entries[buffer->end - 1].valid = false;   
    buffer->tail--;
}

int pnoo_recv_buffer_search_from_rank(const pnoo_recv_buffer_t* buffer, const rank_t src)
{
    for (int i = 0; i < buffer->tail; i++) {
        if (buffer->entries[i].valid && buffer->entries[i].flit.src == src) {
            return i;
        }
    }
    
    return -1;
}

int pnoo_recv_buffer_search_from_any(const pnoo_recv_buffer_t* buffer)
{
    for (int i = 0; i < buffer->tail; i++) {
        if (buffer->entries[i].valid) {
            return i;
        }
    }
    
    return -1;
}

flit_t pnoo_recv_buffer_get_flit(const pnoo_recv_buffer_t* buffer, const int index)
{
    flit_t result;
    memcpy(&result, buffer->entries[index].flit.payload, sizeof(flit_t));
    return result;
}

rank_t pnoo_recv_buffer_get_src(const pnoo_recv_buffer_t* buffer, const int index)
{
    if (index >= 0) {
        return buffer->entries[index].flit.src;
    }
    
    return -1;
}


