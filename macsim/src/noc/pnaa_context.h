/*
 * gs_all_to_all_context.h
 *
 *  Created on: 19.03.2015
 *      Author: gorlorom
 */

#ifndef _PNAA_CONTEXT_H_
#define _PNAA_CONTEXT_H_

#include "share.h"
#include "pbuffer.h"

typedef struct {
	flit_container_t *flit;
}all_to_all_field_t;


//context for node
typedef struct {

	all_to_all_field_t in_west;
	all_to_all_field_t in_south;
	all_to_all_field_t out_east;
	all_to_all_field_t out_north;
	all_to_all_field_t in_core;

	p_buffer_t injectionQueue;
    p_buffer_t core_buffer;

    flit_container_t **corner_buffer;

    uint_fast32_t smallCycle;

    // horizontal
    uint_fast32_t hCycle;
    uint_fast32_t hPhase;
    uint_fast32_t hTimeout;

    // vertical
    uint_fast32_t vTimeout;
    uint_fast32_t vCycle;

} pnaa_context_t;


#endif
