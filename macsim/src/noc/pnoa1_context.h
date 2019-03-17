/*
 * pnoa_context.h
 *
 *  Created on: 16.03.2015
 *      Author: gorlorom
 */

#ifndef _PNOA1_CONTEXT_H_
#define _PNOA1_CONTEXT_H_

#include "share.h"
#include "pbuffer.h"

typedef struct {
    flit_container_t *flit;
} one_to_all_field_t1;


//context for node
typedef struct {

    one_to_all_field_t1 in_west;
    one_to_all_field_t1 in_south;
    one_to_all_field_t1 in_core;
    one_to_all_field_t1 out_east;
    one_to_all_field_t1 out_north;
    one_to_all_field_t1 corner_buffer;

    uint_fast32_t roundTime;
    uint_fast32_t currentCycle;

    p_buffer_t core_buffer;
    p_buffer_t injectionQueue;

    uint64_min_t ready_state[MAX_RANK/64];

} pnoa1_context_t;


#endif


