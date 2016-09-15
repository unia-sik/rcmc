/*
 * pnoa_context.h
 *
 *  Created on: 16.03.2015
 *      Author: gorlorom
 */

#ifndef _PNOA_CONTEXT_H_
#define _PNOA_CONTEXT_H_

#include "share.h"
#include "pbuffer.h"

typedef struct {
    flit_container_t *flit;
} one_to_all_field_t;


//context for node
typedef struct {

    one_to_all_field_t in_west;
    one_to_all_field_t in_south;
    one_to_all_field_t in_core;
    one_to_all_field_t out_east;
    one_to_all_field_t out_north;
    one_to_all_field_t corner_buffer;

    uint_fast32_t roundTime;
    uint_fast32_t currentCycle;

    p_buffer_t core_buffer;
    p_buffer_t injectionQueue;

} pnoa_context_t;


#endif


