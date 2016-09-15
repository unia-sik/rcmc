/*
 * paternoster_best_effort_context.h
 *
 *  Created on: 02.04.2015
 *      Author: gorlorom
 */

#ifndef PATERNOSTER_BEST_EFFORT_CONTEXT_H_
#define PATERNOSTER_BEST_EFFORT_CONTEXT_H_

#include "pbuffer.h"
#include "share.h"

typedef struct {
	flit_container_t *flit;
}pn_best_effort_field_t;

//context for node
typedef struct {

	pn_best_effort_field_t in_west;
	pn_best_effort_field_t in_south;
	pn_best_effort_field_t in_core;
	pn_best_effort_field_t out_east;
	pn_best_effort_field_t out_north;

    p_buffer_t corner_buffer;
    p_buffer_t core_buffer;
    p_buffer_t injectionQueue;

    uint_fast16_t *h_timers;			// pointer to an array, that holds a timer for every column
    uint_fast16_t sendBlock;
    uint_fast16_t receive_timeout;

    bool sentNorth;				// did i send a flit from the corner-buffer last cycle?
    bool sentEast;				// did i send a flit from the core to east?
    bool isSignaledNorth;		// is someone signaling me?
    bool isSignaledEast;		// is someone signaling me?
    bool signalWest;			// wants to signal west for the next round; necessary because the program is sequential
    bool signalSouth;			// wants to signal south for the next round;

} pnbe2_context_t;



#endif /* PATERNOSTER_BEST_EFFORT_CONTEXT_H_ */
