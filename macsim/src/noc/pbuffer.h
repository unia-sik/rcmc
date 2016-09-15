/*
 * pbuffer.h
 *
 *  Created on: 17.02.2015
 *      Author: gorlorom
 *
 *	The pbuffer provides buffer utility for flits in all paternoster implementations.
 */

#ifndef PBUFFER_H_
#define PBUFFER_H_

#include <stdbool.h>
#include "share.h"

#define CORNER_BUFFER_SIZE	8	// old

typedef struct p_buffer_entry_s{
	flit_container_t *flit;
	struct p_buffer_entry_s *next;
}p_buffer_entry_t;

typedef struct{
	p_buffer_entry_t *head;
	p_buffer_entry_t *tail;
	uint_fast32_t slots;
	uint_fast32_t size;
}p_buffer_t;

// add a message as last to the buffer
bool msgenque(p_buffer_t *buffer, flit_container_t *flit);

//get the first message from the buffer
flit_container_t *msgdeque(p_buffer_t *buffer);

//get the first message received from rank
flit_container_t *msgdeque_rank(p_buffer_t *buffer, rank_t rank);

//initialize a buffer with a maximum size
void buffer_init(p_buffer_t *buffer, uint_fast8_t size);

#endif /* PBUFFER_H_ */
