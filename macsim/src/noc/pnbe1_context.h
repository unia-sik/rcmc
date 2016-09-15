/*
 * best_effort_prototype1_context.h
 *
 *  Created on: 09.04.2015
 *      Author: gorlorom
 */

#ifndef BEST_EFFORT_PROTOTYPE1_CONTEXT_H_
#define BEST_EFFORT_PROTOTYPE1_CONTEXT_H_

#include "pbuffer.h"
#include "share.h"

typedef struct {
	flit_container_t *flit;
}p1_best_effort_field_t;

//context for node
typedef struct {

	p1_best_effort_field_t in_west;
	p1_best_effort_field_t in_south;
	p1_best_effort_field_t in_core;
	p1_best_effort_field_t out_east;
	p1_best_effort_field_t out_north;

    p_buffer_t corner_buffer;
    p_buffer_t core_buffer;
    p_buffer_t injectionQueue;

    uint_fast16_t *h_timers;			// pointer to an array, that holds a timer for every column

    bool sentNorth;				// did i send a flit from the corner-buffer last cycle?
    bool sentEast;				// did i send a flit from the core to east?
    bool isSignaledNorth;		// is someone signaling me?
    bool isSignaledEast;		// is someone signaling me?
    bool signalWest;			// wants to signal west for the next round; necessary because the program is sequential
    bool signalSouth;			// wants to signal south for the next round;

} pnbe1_context_t;



#endif /* BEST_EFFORT_PROTOTYPE1_CONTEXT_H_ */
