/*
 * paternoster_context.h
 *
 *  Created on: 16.02.2015
 *      Author: gorlorom
 */

#ifndef _PNBE0_CONTEXT_H_
#define _PNBE0_CONTEXT_H_
#include "pbuffer.h"
#include "share.h"

typedef struct {
    flit_container_t *flit;
}msg_field_t;

void msg_field_clear(msg_field_t *field);

//context for node
typedef struct {

    msg_field_t paternoster_in_west;
    msg_field_t paternoster_in_south;
    msg_field_t paternoster_in_core;
    msg_field_t paternoster_out_east;
    msg_field_t paternoster_out_north;

    p_buffer_t corner_buffer;
    p_buffer_t core_buffer;
    p_buffer_t injectionQueue;

    rank_t *h_timers;			// pointer to an array, that holds a timer for every column
    rank_t sendBlock;			// rank_t in order to avoid problems if the datatype of rank_t is changed
    rank_t receive_timeout;

    bool sentNorth;				// did i send a flit from the corner-buffer last cycle?
    bool sentEast;				// did i send a flit from the core to east?
    bool isSignaledNorth;		// is someone signaling me?
    bool isSignaledEast;		// is someone signaling me?
    bool signalWest;			// wants to signal west for the next round; necessary because the program is sequential
    bool signalSouth;			// wants to signal south for the next round;

} pnbe0_context_t;

#endif
