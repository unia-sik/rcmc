/*
 * caerus_context.h
 *
 *  Created on: 31.03.2015
 *      Author: gorlorom
 */

#ifndef CAERUS_CONTEXT_H_
#define CAERUS_CONTEXT_H_

#include "share.h"
#include "pbuffer.h"

typedef struct {
	flit_container_t *flit;
}caerus_field_t;

typedef struct {

	caerus_field_t in_west;
	caerus_field_t in_south;
	caerus_field_t out_north;
	caerus_field_t out_east;
	caerus_field_t in_core;

	p_buffer_t injectionQueue;
	p_buffer_t core_buffer;

	p_buffer_t corner_buffer;
	bool cornerON;
	uint_fast16_t cornerCounter;

	uint_fast16_t *hTimeouts;

	bool isSignaledEast;
	bool signalWest;
	bool sentEast;


} caerus_context_t;




#endif /* CAERUS_CONTEXT_H_ */
